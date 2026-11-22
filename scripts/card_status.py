#!/usr/bin/env python3
"""Per-card implementation status against the official card list.

scripts/card_coverage.py answers "does this file contain effect code?". That is
a build gate, not a status: a card with one clause of three wired passes it.
This script starts from every card in cards/card_index.json and asks, per card:

  1. Is there a class at all?                      (no -> not-implemented)
  2. Does the class do anything?                   (no, but it has rules text -> not-implemented)
  3. Does a comment in the file admit a gap?       (yes -> partial-documented)
  4. Has a human read it and found it wrong?       (yes -> partial-confirmed)
  5. Does every printed clause have matching code? (no -> partial-suspect)
  6. Is the card referenced by a test?             (implemented-tested / -untested)

Step 5 is a static heuristic: each printed verb or trigger is looked up in the
class body. A hit proves code mentions the concept, not that it is correct, and
a miss can be a helper the regex does not know. It is a triage for a clause-level
review, not a verdict.

Writes docs/card-status/card-status.csv and docs/card-status/README.md.

Usage:  python3 scripts/card_status.py
"""
import csv
import json
import re
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CARDS = ROOT / "src/cards"
OUT = ROOT / "docs/card-status"

ORDER = ["not-implemented", "partial-confirmed", "partial-documented", "partial-suspect",
         "implemented-untested", "implemented-tested", "vanilla"]
NOT_DONE = ORDER[:4]

# Verdicts from reading the card against its class. Keep a verdict here until
# the card is fixed; delete it in the commit that fixes the card.
CONFIRMED = {}
# Flagged by the heuristic, read, and found correct.
REVIEWED_OK = {
    12, 25, 79, 150, 161, 165, 170, 174, 176, 193, 194, 231, 236, 243, 250, 260, 264, 273,
    286, 308, 341, 347, 358, 380, 384, 415, 449, 462, 463, 547, 587, 648, 693, 709, 726,
    728, 729, 731, 732, 740, 762, 809, 823, 873, 876, 885, 900,
}

# A comment admitting the card does less than it prints. Deliberately narrower
# than card_coverage.py's markers, which also match "cost cannot be paid".
GAP = re.compile(
    r"(?i)(engine gap|engine limitation|approximat|unimplemented|not implemented|"
    r"\binert\b|\btodo\b|\bfixme\b|best-effort|cannot be (wired|expressed|enforced)|"
    r"cannot fire|no engine (hook|trigger|site|primitive)|has no engine|"
    r"no structured|not plumbed|needs engine|requires? engine|is not modelled)")
# Comments that match GAP but describe something already fixed or not a gap.
GAP_NOT = re.compile(r"(?i)(was approximated|power cost approximated as free|no engine behind it|does not yet fire in tests)")

# Printed verb -> code that would have to exist. Applied to rules text with
# reminder text removed, and matched against the class body without its CardDef.
CLAUSES = {
    "draw":     (r"\bdraw\b",                        r"(?i)draw"),
    "discard":  (r"\bdiscard\b",                     r"(?i)discard"),
    "kill":     (r"\bkill\b",                        r"(?i)kill"),
    "banish":   (r"\bbanish\b",                      r"(?i)banish"),
    "stun":     (r"\bstun\b",                        r"(?i)stun"),
    "recycle":  (r"\brecycle\b",                     r"(?i)recycle"),
    "channel":  (r"\bchannel\b",                     r"(?i)channel"),
    "buff":     (r"\bbuff\b",                        r"(?i)buff"),
    "ready":    (r"\bready (me|it|this|a |an |your|up to|that|them|all)", r"(?i)ready|exhausted\s*=\s*false"),
    "exhaust":  (r"\bexhaust (a |an |up to|target|that|it|all)", r"(?i)exhaust"),
    "damage":   (r"\bdeal\b",                        r"(?i)damage"),
    "heal":     (r"\bheal\b",                        r"(?i)heal|damage\s*=\s*0"),
    "score":    (r"\bscore \d|\bgain \d+ points?\b", r"(?i)score|point"),
    "bounce":   (r"\breturn\b.*\bhand\b",            r"(?i)bounce|tohand|returnto|hand\.push_back"),
    "recall":   (r"\brecall\b",                      r"(?i)recall|moveToBase"),
    "move":     (r"\bmove (a |an |me|it|up to|any|target|that|all|your)", r"(?i)move"),
    "token":    (r"\btokens?\b",                     r"(?i)token|createGold"),
    "counter":  (r"\bcounter (a |an |target|that|it)", r"(?i)counter"),
    "reveal":   (r"\breveal\b",                      r"(?i)reveal"),
    "look":     (r"\blook at\b",                     r"(?i)look|reveal|main_deck"),
    "shuffle":  (r"\bshuffle\b",                     r"(?i)shuffle"),
    "xp":       (r"\bgain \d* ?xp\b|\bspend \d* ?xp\b", r"(?i)\bxp\b|\.xp|xp\(|gainXp"),
    "might":    (r"(?<!have )[+-]\d+ \[m\]",          r"(?i)might|aura"),
    "control":  (r"\bgain control\b",                r"(?i)control"),
    "predict":  (r"\[predict",                       r"(?i)predict"),
    "add-rune": (r"\badd \[",                        r"(?i)rune_pool|addEnergy|addPower|add[A-Z]\w*\("),
    # Trigger clauses: the class must register for the event.
    "on-play":  (r"\bwhen you play (me|this)\b",     r"WhenYouPlay(Me|This)|onPlay|AsYouPlay"),
    "on-attack":(r"\bwhen i attack\b",               r"Attack"),
    "on-defend":(r"\bwhen i defend\b",               r"Defend"),
    "on-conquer":(r"\bwhen (i|you) conquer\b",       r"Conquer"),
    "on-hold":  (r"\bwhen (i|you) hold\b",           r"Hold"),
    "on-die":   (r"(^|\n)\[deathknell\]|\bwhen i die\b", r"Die|Death|Deathknell|onDeath"),
    "on-move":  (r"\bwhen i move\b",                 r"Move"),
    "end-turn": (r"\bat the end of (your|this|each)\b", r"AtEndOfTurn|EndOfTurn|Delayed|delayed|End"),
    "start-turn":(r"\bat the start of (your|each|the next)\b", r"AtStartOf|Delayed|delayed|Start"),
    "empower":  (r"\[empower\]",                     r"empowerSpec|mpower"),
    "flow":     (r"\[flow\] ",                       r"(?i)flow|TrashReplayGrant"),
    "burn":     (r"\bburn \d",                       r"(?i)burn"),
    "legion":   (r"\[legion\]",                      r"(?i)legion|cards_played_this_turn"),
    "activated":(r"(^|\n)(\[[^\]]+\],? ?)+:",        r"activatedAbilities|hasActivatedAbility|onActivate|getActivationCost|empowerSpec"),
}
# Printed decisions. A card that auto-picks hides the decision from search, which
# then cannot find the line at all, so this is checked apart from the verbs.
CHOICE_PRINTED = r"\byou may\b|\bchoose (?!an opponent)"   # one opponent in 1v1
CHOICE_CODE = (r"confirmOptional|pickMode|pickTarget|pickXAmount|requestChoice|Choice|"
               r"getTargetRequirements|enumerateLegalTargets|optionalAdditionalCost|"
               r"alternativePlayCost|getPlayLocations|pick[A-Z]")

# Keywords with no card code by design: the engine reads the keyword bit.
ENGINE_KEYWORDS = re.compile(
    r"(?i)\[(accelerate|action|ambush|assault( \d+)?|backline|deflect( \d+)?|equip|"
    r"ganking|hidden|quick-?draw|reaction|shield( \d+)?|tank|temporary|unique|"
    r"vision|weaponmaster|repeat)[^\]]*\]")

CODE_ARTIFACT_RE = re.compile(r'R"RB\(.*?\)RB"', re.S)
DEF_BLOCK_RE = re.compile(r"const CardDef def_ = \[\] \{.*?\}\(\);", re.S)
FIELD_RE = {k: re.compile(rf'd\.{k} = R"RB\((.*?)\)RB";', re.S)
            for k in ("def_id", "name", "ability_text", "effect_text", "set_code")}
INT_RE = {k: re.compile(rf"d\.{k} = (-?\d+);") for k in ("id", "energy_cost", "power_cost", "might", "might_bonus")}


def reminderless(text):
    """Rules text without reminder parentheticals."""
    prev = None
    while prev != text:
        prev, text = text, re.sub(r"\([^()]*\)", "", text)
    return text


def norm(text):
    """Text comparison key: ignores case, whitespace and glyph differences."""
    text = (text or "").replace("—", "-").replace("’", "'").lower()
    text = re.sub(r"[^a-z0-9\[\]+\-]+", " ", reminderless(text))
    return " ".join(text.split())


def load_classes():
    classes = {}
    for path in sorted(CARDS.rglob("[0-9]*.cpp")):
        src = path.read_text(encoding="utf-8", errors="replace")
        field = lambda k: (m.group(1) if (m := FIELD_RE[k].search(src)) else "")
        num = lambda k: (int(m.group(1)) if (m := INT_RE[k].search(src)) else None)
        cid = num("id") or int(path.name.split("_")[0])
        body = DEF_BLOCK_RE.sub("", src)
        no_strings = CODE_ARTIFACT_RE.sub('""', body)
        comments = re.findall(r"//[^\n]*", no_strings)
        code = re.sub(r"//[^\n]*", "", no_strings)
        overrides = [o for o in re.findall(r"\b(\w+)\s*\([^;{}]*\)[^;{}]*\boverride\b", code)
                     if o != "def"]
        gaps = [c.lstrip("/ ").strip() for c in comments
                if GAP.search(c) and not GAP_NOT.search(c)]
        classes[cid] = dict(
            id=cid, file=str(path.relative_to(ROOT)).replace("\\", "/"),
            def_id=field("def_id"), name=field("name"), set=field("set_code"),
            text=field("ability_text"), effect=field("effect_text"),
            energy=num("energy_cost"), power=num("power_cost"),
            might=num("might") or num("might_bonus"),
            code=code, overrides=overrides, gaps=gaps,
            banned="banned = true" in src, coverage_ok="// COVERAGE-OK" in src,
            base=(m.group(1) if (m := re.search(r"class \w+ : public (\w+)", code)) else ""),
        )
    return classes


def test_references(classes):
    """Ids and names referenced anywhere under tests/."""
    ids, blob = set(), []
    for path in (ROOT / "tests").rglob("*"):
        if path.suffix not in (".cpp", ".h") or path.name == "card_test_fixture.h":
            continue
        src = path.read_text(encoding="utf-8", errors="replace")
        blob.append(src)
        ids.update(int(n) for n in re.findall(r"\bk\w+\s*=\s*(\d{1,4})\b", src))
        ids.update(int(n) for n in re.findall(
            r"(?:CardDefId|addToHand|addUnit|addCard|addGear|addToTrash|addToDeck|"
            r"pushSpellOnChain|registry\.get|card_db\.get|getCard)\([^()]*?\b(\d{1,4})\b", src))
    blob = "\n".join(blob)
    return {cid for cid, c in classes.items()
            if cid in ids or (c["name"] and re.search(re.escape(c["name"].split(",")[0]) + r"\b", blob)
                              and len(c["name"]) > 5 and c["name"] in blob)}


def deck_usage(classes):
    by_name = defaultdict(list)
    for c in classes.values():
        by_name[c["name"].lower()].append(c["id"])
    copies = Counter()
    for path in (ROOT / "decks").glob("*.txt"):
        for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
            m = re.match(r"\s*(\d+)\s*x?\s+(.+?)\s*$", line)
            if m and m.group(2).lower() in by_name:
                copies[by_name[m.group(2).lower()][0]] += int(m.group(1))
    return copies


def missing_clauses(card):
    rules = reminderless(card["text"] + "\n" + card["effect"]).lower()
    # [Quick-Draw], [Repeat] and friends are not verbs.
    verbs = ENGINE_KEYWORDS.sub("", rules)
    missing = [name for name, (printed, needed) in CLAUSES.items()
               if re.search(printed, verbs) and not re.search(needed, card["code"])]
    if re.search(CHOICE_PRINTED, verbs) and not re.search(CHOICE_CODE, card["code"]):
        missing.append("decision")
    return missing


def classify(card, tested):
    """(status, reason) for one class."""
    residual = ENGINE_KEYWORDS.sub("", reminderless(card["text"] + " " + card["effect"]))
    has_rules = len(re.sub(r"[^a-zA-Z]", "", residual)) >= 4
    if not card["overrides"] and not card["coverage_ok"] and has_rules:
        return "not-implemented", "rules text but no behaviour in the class"
    if card["id"] in CONFIRMED:
        return "partial-confirmed", CONFIRMED[card["id"]]
    if card["gaps"]:
        return "partial-documented", card["gaps"][0][:110]
    missing = missing_clauses(card)
    if missing and not card["coverage_ok"] and card["id"] not in REVIEWED_OK:
        return "partial-suspect", "no code for: " + ", ".join(missing)
    if not has_rules:
        return "vanilla", "keywords and stats only"
    if card["id"] in tested:
        return "implemented-tested", ""
    return "implemented-untested", ""


def main():
    index = json.loads((ROOT / "cards/card_index.json").read_text(encoding="utf-8"))
    classes = load_classes()
    tested = test_references(classes)
    decks = deck_usage(classes)
    by_def = {c["def_id"]: c for c in classes.values()}
    by_name = {c["name"].lower(): c for c in classes.values()}

    rows, seen = [], set()
    for official in index:
        card = by_def.get(official["id"]) or by_name.get(official["name"].lower())
        if card is None:
            rows.append(dict(id="", code=official["public_code"], name=official["name"],
                             set=official["set"], type=official["card_type"],
                             status="not-implemented", reason="no class in src/cards",
                             tested=False, deck_copies=0, banned=False, text_drift=False,
                             stat_drift="", file=""))
            continue
        if card["id"] in seen:
            continue    # reprint of a card already listed
        seen.add(card["id"])
        status, reason = classify(card, tested)
        printed = norm((official.get("ability_text") or "") + " " + (official.get("effect_text") or ""))
        stats = [f"{k} {card[k2]}!={official[k]}"
                 for k, k2 in (("energy_cost", "energy"), ("power_cost", "power"), ("might", "might"))
                 if (official.get(k) or 0) != (card[k2] or 0)]
        rows.append(dict(id=card["id"], code=official["public_code"], name=card["name"],
                         set=official["set"], type=official["card_type"], status=status,
                         reason=reason, tested=card["id"] in tested,
                         deck_copies=decks[card["id"]], banned=card["banned"],
                         text_drift=printed != norm(card["text"] + " " + card["effect"]),
                         stat_drift="; ".join(stats), file=card["file"]))
    for card in classes.values():      # classes with no official entry (tokens, promos)
        if card["id"] not in seen:
            status, reason = classify(card, tested)
            rows.append(dict(id=card["id"], code=card["def_id"], name=card["name"],
                             set=card["set"], type="?", status=status, reason=reason,
                             tested=card["id"] in tested, deck_copies=decks[card["id"]],
                             banned=card["banned"], text_drift=False, stat_drift="",
                             file=card["file"]))

    rows.sort(key=lambda r: (ORDER.index(r["status"]), -r["deck_copies"], r["id"] or 9999, r["name"]))
    OUT.mkdir(parents=True, exist_ok=True)
    with open(OUT / "card-status.csv", "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0]), lineterminator="\n")
        w.writeheader()
        w.writerows(rows)
    write_report(rows)


def table(rows, cols):
    head = "| " + " | ".join(c for c, _ in cols) + " |\n|" + "---|" * len(cols) + "\n"
    return head + "".join(
        "| " + " | ".join(str(r[k]).replace("|", "/") for _, k in cols) + " |\n" for r in rows)


def write_report(rows):
    total = Counter(r["status"] for r in rows)
    by_set = defaultdict(Counter)
    for r in rows:
        by_set[r["set"]][r["status"]] += 1
    in_decks = [r for r in rows if r["deck_copies"]]
    deck_bad = [r for r in in_decks if r["status"] in NOT_DONE]

    out = ["# Card implementation status", "",
           "Generated by `scripts/card_status.py`; do not edit by hand. Full data, one row per "
           "card: [card-status.csv](card-status.csv).", "",
           "**This is a static triage, not a verification.** `partial-suspect` means a printed "
           "verb, trigger or decision has no matching code in the class; `implemented-*` means "
           "the check found nothing wrong, not that the card is correct.", "",
           "## Totals", "",
           "| Status | Cards | Meaning |", "|---|---|---|"]
    meaning = {
        "not-implemented": "No class, or a class with rules text and no behaviour",
        "partial-confirmed": "Read against the card text and found wrong or incomplete",
        "partial-documented": "Implemented, but a comment in the file admits a gap",
        "partial-suspect": "A printed clause has no matching code (heuristic)",
        "implemented-untested": "All clauses have code; no test references the card",
        "implemented-tested": "All clauses have code; a test references the card",
        "vanilla": "Keywords and stats only; no card code needed",
    }
    out += [f"| {s} | {total[s]} | {meaning[s]} |" for s in ORDER]
    out += [f"| **total** | **{len(rows)}** | |", "", "## By set", "",
            "| Set | " + " | ".join(ORDER) + " |", "|---|" + "---|" * len(ORDER)]
    out += [f"| {s} | " + " | ".join(str(by_set[s][o]) for o in ORDER) + " |" for s in sorted(by_set)]
    out += ["", f"## Sample decks: {len(deck_bad)} of {len(in_decks)} distinct cards in `decks/` "
            "are not fully implemented", "",
            "Fix these first: they affect every evaluation of those decks.", "",
            table(deck_bad, [("id", "id"), ("name", "name"), ("copies", "deck_copies"),
                             ("status", "status"), ("reason", "reason")]).rstrip(), ""]
    for status, cols in (
        ("not-implemented", [("code", "code"), ("name", "name"), ("type", "type"), ("reason", "reason")]),
        ("partial-confirmed", [("id", "id"), ("name", "name"), ("copies", "deck_copies"), ("what is wrong", "reason")]),
        ("partial-documented", [("id", "id"), ("name", "name"), ("copies", "deck_copies"), ("first admitted gap", "reason")]),
        ("partial-suspect", [("id", "id"), ("name", "name"), ("copies", "deck_copies"), ("reason", "reason")]),
    ):
        part = [r for r in rows if r["status"] == status]
        out += [f"## {status} ({len(part)})", "", table(part, cols).rstrip(), ""]
    drift = [r for r in rows if r["text_drift"] or r["stat_drift"]]
    out += [f"## Differences from the official data ({len(drift)})", "",
            "The class's text or stats differ from `cards/card_index.json` after normalising "
            "reminder text and glyphs. The gallery can lag the errata documents, so each needs "
            "adjudicating against `errata/` before behaviour changes.", "",
            table(drift, [("id", "id"), ("name", "name"), ("stat diff", "stat_drift"),
                          ("text differs", "text_drift"), ("status", "status")]).rstrip(), ""]
    (OUT / "README.md").write_text("\n".join(out), encoding="utf-8", newline="\n")

    for s in ORDER:
        print(f"{s:22}{total[s]:5}")
    print(f"{'total':22}{len(rows):5}")
    print(f"sample-deck cards not fully implemented: {len(deck_bad)}/{len(in_decks)}")
    print(f"official-data differences: {len(drift)}")


if __name__ == "__main__":
    main()
