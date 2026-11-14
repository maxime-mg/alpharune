// Riftbound visual board (Phase 1, PLAY-010/011/012).
// Plain script, no build step. Reads the per-seat `board` view the server
// sends in every frame and draws it. Actions are the engine's own legal
// intents: clicking one sends its index back, exactly as the classic page does.
(function () {
  "use strict";

  const $ = (id) => document.getElementById(id);
  const el = (tag, cls, text) => {
    const e = document.createElement(tag);
    if (cls) e.className = cls;
    if (text !== undefined && text !== null) e.textContent = text;
    return e;
  };

  const DOMAIN_VAR = {
    Fury: "--fury", Calm: "--calm", Mind: "--mind", Body: "--body",
    Chaos: "--chaos", Order: "--order", Colorless: "--colorless",
  };
  const PLAYER_LABEL = { P1: "Player 1", P2: "Player 2" };
  const MAX_LOG = 400;

  // ── State ──
  let ws = null;
  let frame = null;          // last decision/state frame
  let retryMs = 500;
  const seatParam = new URLSearchParams(location.search).get("seat") || "auto";
  const objIndex = new Map(); // object id -> card json, rebuilt each frame
  let highlightIds = new Set();
  // A card picked on the board that still needs a destination:
  // { card, name, acts } where acts are the options that place it.
  let placing = null;

  // ── Seat selector ──
  const seatSelect = $("seat-select");
  seatSelect.value = seatParam;
  seatSelect.addEventListener("change", () => {
    const u = new URL(location.href);
    u.searchParams.set("seat", seatSelect.value);
    location.href = u.toString();
  });

  // ── Text view toggle ──
  const textToggle = $("text-toggle");
  try { textToggle.checked = localStorage.getItem("rb.textView") === "1"; } catch (_) {}
  textToggle.addEventListener("change", () => {
    try { localStorage.setItem("rb.textView", textToggle.checked ? "1" : "0"); } catch (_) {}
    renderTextView();
  });

  $("jump-actions").addEventListener("click", () =>
    $("decision-panel").scrollIntoView({ behavior: "smooth", block: "start" }));

  // ── Modal ──
  $("modal-close").addEventListener("click", closeModal);
  $("modal").addEventListener("click", (e) => { if (e.target.id === "modal") closeModal(); });
  document.addEventListener("keydown", (e) => {
    if (e.key !== "Escape") return;
    if (!$("modal").hidden) closeModal();
    else if (placing) cancelPlacing();
  });
  function openModal(title, cards, emptyText) {
    $("modal-title").textContent = title;
    const body = $("modal-body");
    body.replaceChildren();
    if (!cards || cards.length === 0) body.append(el("div", "waiting", emptyText || "Empty"));
    for (const c of cards || []) body.append(cardEl(c, "hand"));
    $("modal").hidden = false;
  }
  function closeModal() { $("modal").hidden = true; hideTooltip(); }

  // ── Connection ──
  function connect() {
    const proto = location.protocol === "https:" ? "wss" : "ws";
    ws = new WebSocket(`${proto}://${location.host}/ws?seat=${encodeURIComponent(seatParam)}`);
    ws.onopen = () => {
      retryMs = 500;
      setConn(true);
      ws.send(JSON.stringify({ type: "request_state" }));
    };
    ws.onclose = () => {
      setConn(false);
      setTimeout(connect, retryMs);
      retryMs = Math.min(retryMs * 2, 5000);
    };
    ws.onmessage = (ev) => {
      let m;
      try { m = JSON.parse(ev.data); } catch (_) { return; }
      if (m.type === "decision" || m.type === "state") { frame = m; placing = null; render(); }
      else if (m.type === "log") appendLog(m);
      else if (m.type === "err" || m.type === "edit_err") flash(m.msg || "error");
    };
  }
  function setConn(on) {
    const c = $("conn");
    c.textContent = on ? "connected" : "reconnecting";
    c.className = "conn " + (on ? "conn-on" : "conn-off");
  }
  function send(obj) { if (ws && ws.readyState === 1) ws.send(JSON.stringify(obj)); }

  function choose(index) {
    if (!frame) return;
    send({ type: "choose", player: frame.player, index, decision_id: frame.decision_id });
    // Disable the buttons until the next frame arrives, so a double click
    // cannot answer the following decision.
    for (const b of $("actions").querySelectorAll("button")) b.disabled = true;
  }

  function flash(msg) {
    const p = $("prompt");
    p.hidden = false;
    p.textContent = "⚠ " + msg;
  }

  // ── Helpers ──
  const board = () => (frame && frame.board) || null;
  // Who sits at the bottom: my player, or P1 for spectator and god.
  const bottomPlayer = () => (board() && board().viewer_player) || "P1";
  const topPlayer = () => (bottomPlayer() === "P1" ? "P2" : "P1");
  const isMine = (p) => board() && board().viewer_player === p;
  const sideClass = (p) => (board() && board().viewer_player
    ? (isMine(p) ? "mine" : "theirs")
    : (p === "P1" ? "mine" : "theirs"));
  const playerName = (p) => (PLAYER_LABEL[p] || p) + (isMine(p) ? " (you)" : "");

  function domainVar(card) {
    const d = (card.domains || [])[0];
    return d && DOMAIN_VAR[d] ? `var(${DOMAIN_VAR[d]})` : "var(--colorless)";
  }

  function indexObjects(b) {
    objIndex.clear();
    const add = (c) => {
      if (!c || c.back) return;
      if (typeof c.id === "number") objIndex.set(c.id, c);
      for (const a of c.attachments || []) add(a);
    };
    for (const p of Object.values(b.players)) {
      add(p.legend); add(p.champion);
      for (const z of ["base", "runes", "trash", "banishment"]) for (const c of p[z] || []) add(c);
      for (const c of (p.hand && p.hand.cards) || []) add(c);
      for (const c of (p.main_deck && p.main_deck.cards) || []) add(c);
    }
    for (const bf of b.battlefields) {
      add(bf.card);
      for (const side of Object.values(bf.units)) for (const c of side) add(c);
      for (const c of bf.facedown) add(c);
    }
  }

  // Every object an action names: the card played, the ability source,
  // and its targets or chosen objects.
  function actionIds(a) {
    return [a.card_object_id, a.ability_source, ...(a.objects || [])].filter((x) => typeof x === "number");
  }

  // Legal actions grouped by the object they act on, so a card can show
  // that it has an action and clicking it can offer exactly those.
  function actionsByObject() {
    const m = new Map();
    for (const a of (frame && frame.legal) || []) {
      for (const id of actionIds(a)) {
        if (typeof id !== "number") continue;
        if (!m.has(id)) m.set(id, []);
        if (!m.get(id).includes(a)) m.get(id).push(a);
      }
    }
    return m;
  }

  // ── Card element ──
  function cardEl(card, size, actionMap) {
    if (!card) return el("div", "slot", "—");
    if (card.back) {
      const b = el("div", "card back" + (size ? " " + size : ""));
      if (card.controller) b.classList.add(sideClass(card.controller));
      b.title = "Face-down card";
      return b;
    }
    if (card.type === "Rune") {
      const r = el("div", "card rune" + (card.exhausted ? " exhausted" : ""));
      if (typeof card.id === "number") r.dataset.obj = card.id;
      const d = (card.rune_domains || card.domains || [])[0] || "";
      r.style.setProperty("--dom", domainVar(card));
      r.textContent = d ? d[0] : "R";
      attachTooltip(r, card);
      markActionable(r, card, actionMap);
      return r;
    }
    const c = el("div", "card" + (size ? " " + size : ""));
    if (typeof card.id === "number") c.dataset.obj = card.id;  // for hover highlight
    c.style.setProperty("--dom", domainVar(card));
    if (card.image) c.style.backgroundImage = `url("${card.image}")`;
    else c.classList.add("no-art");
    if (card.exhausted) c.classList.add("exhausted");
    if (highlightIds.has(card.id)) c.classList.add("highlight");

    if (card.energy_cost !== undefined && ["Unit", "Spell", "Gear"].includes(card.type)) {
      const cost = el("div", "cost", String(card.energy_cost) + (card.power_cost ? "+" + card.power_cost : ""));
      c.append(cost);
    }
    if (card.might !== undefined) {
      const m = el("div", "might", String(card.might));
      if (card.might > card.base_might) m.classList.add("up");
      if (card.might < card.base_might) m.classList.add("down");
      m.title = `Might ${card.might} (base ${card.base_might})`;
      c.append(m);
      if (card.damage > 0) c.append(el("div", "dmg", `-${card.damage}`));
    }
    const tags = el("div", "tags");
    if (card.designation === "Attacker") tags.append(el("span", "tag atk", "ATK"));
    if (card.designation === "Defender") tags.append(el("span", "tag def", "DEF"));
    if (card.stunned) tags.append(el("span", "tag stun", "STUN"));
    if (card.buffs > 0) tags.append(el("span", "tag buff", `+${card.buffs}`));
    if (card.token) tags.append(el("span", "tag token", "TOKEN"));
    if (card.hidden) tags.append(el("span", "tag hidden", "HIDDEN"));
    if (card.empowered) tags.append(el("span", "tag buff", "EMP"));
    if (tags.childElementCount) c.append(tags);

    if ((card.keywords || []).length && size !== "small") {
      const k = el("div", "kws");
      for (const kw of card.keywords.slice(0, 4))
        k.append(el("span", "kw", kw.value ? `${kw.name} ${kw.value}` : kw.name));
      c.append(k);
    }
    c.append(el("div", "name", card.name));
    attachTooltip(c, card);
    markActionable(c, card, actionMap);

    if ((card.attachments || []).length) {
      const host = el("div", "host");
      const att = el("div", "attachments");
      for (const a of card.attachments) att.append(cardEl(a, "small", actionMap));
      host.append(c, att);
      return host;
    }
    return c;
  }

  // Options that put `card` somewhere: playing it to a location, or moving it.
  function placements(card, acts) {
    return acts.filter((a) => a.location && (a.card_object_id === card.id ||
      (/StandardMove/.test(a.type || "") && (a.objects || []).includes(card.id))));
  }
  const locKey = (loc) => (loc.battlefield !== undefined ? "bf" + loc.battlefield : "base" + loc.base);

  function markActionable(node, card, actionMap) {
    if (!actionMap) return;
    const acts = actionMap.get(card.id);
    if (!acts || !acts.length) { markIdle(node, card); return; }
    node.classList.add("actionable");
    if (placing && placing.card === card.id) node.classList.add("selected");
    node.addEventListener("click", (e) => {
      e.stopPropagation();
      // A card that can go to more than one place (a unit played to base
      // or a battlefield, a unit moving to one of several destinations):
      // select it, then click where it goes. Clicking it again cancels.
      const place = placements(card, acts);
      const spots = new Set(place.map((a) => locKey(a.location)));
      if (place.length && (spots.size > 1 || place.length < acts.length)) {
        if (placing && placing.card === card.id) { cancelPlacing(); return; }
        placing = { card: card.id, name: card.name, acts: place };
        render();
        highlightActions(acts);
        return;
      }
      // One click acts only when this card is what the option is about
      // (the card played, the ability's source, the unit moved) or the
      // decision is a pick among objects. A card that is merely a target
      // highlights the options instead, so a stray click cannot cast a spell.
      if (acts.length === 1) {
        const a = acts[0];
        const primary = a.card_object_id === card.id || a.ability_source === card.id ||
          place.length === 1;
        const pick = /MakeChoice|Choose/.test(a.type || "");
        if (primary || pick) { choose(a.index); return; }
      }
      highlightActions(acts);
    });
  }

  // A unit of the deciding player with nothing to do: say why, since the
  // usual reason (it is exhausted) is easy to miss.
  function markIdle(node, card) {
    if (!frame || frame.type !== "decision" || !(frame.legal || []).length) return;
    if (card.type !== "Unit" || card.controller !== frame.player) return;
    node.addEventListener("click", (e) => {
      e.stopPropagation();
      flash(card.exhausted
        ? `${card.name} is exhausted, so it can't move or attack until it readies. Units enter play exhausted.`
        : `${card.name} has no option right now. Units move only in your main phase while the state is open.`);
    });
  }

  // Make a base zone or battlefield box a destination for the selected card.
  function markDestination(node, loc) {
    if (!placing) return;
    const hits = placing.acts.filter((a) => locKey(a.location) === locKey(loc));
    if (!hits.length) return;
    node.classList.add("drop-target");
    node.addEventListener("click", (e) => {
      e.stopPropagation();
      // Several options for one spot (the printed cost or an alternative
      // cost, an extra cost to play there): let the buttons decide.
      if (hits.length === 1) { const a = hits[0]; cancelPlacing(false); choose(a.index); }
      else highlightActions(hits);
    });
  }

  function cancelPlacing(redraw = true) {
    placing = null;
    if (redraw) render();
  }

  function highlightActions(acts) {
    const want = new Set(acts.map((a) => a.index));
    let first = null;
    for (const b of $("actions").querySelectorAll("button.action")) {
      const hit = want.has(Number(b.dataset.index));
      b.classList.toggle("highlight", hit);
      if (hit && !first) first = b;
    }
    if (first) first.scrollIntoView({ block: "nearest", behavior: "smooth" });
  }

  // ── Tooltip ──
  const tooltip = $("tooltip");
  function attachTooltip(node, card) {
    node.addEventListener("mouseenter", (e) => showTooltip(card, e));
    node.addEventListener("mousemove", positionTooltip);
    node.addEventListener("mouseleave", hideTooltip);
  }
  function showTooltip(card, e) {
    tooltip.replaceChildren();
    if (card.image) { const img = el("img"); img.src = card.image; img.alt = ""; tooltip.append(img); }
    tooltip.append(el("div", "tt-name", card.name));
    const sub = [card.type, (card.domains || []).join("/")];
    if (card.energy_cost !== undefined) sub.push(`cost ${card.energy_cost}${card.power_cost ? " + " + card.power_cost + " power" : ""}`);
    if (card.might !== undefined) sub.push(`might ${card.might}/${card.base_might}`);
    tooltip.append(el("div", "tt-sub", sub.filter(Boolean).join(" · ")));
    if (card.text) tooltip.append(el("div", "tt-text", card.text));
    const state = [];
    if (card.exhausted) state.push("exhausted");
    if (card.damage) state.push(`${card.damage} damage`);
    if (card.stunned) state.push("stunned");
    if (card.buffs) state.push(`${card.buffs} buff(s)`);
    if (card.temp_might) state.push(`${card.temp_might > 0 ? "+" : ""}${card.temp_might} might this turn`);
    if (card.designation) state.push(card.designation.toLowerCase());
    if ((card.keywords || []).length) state.push(card.keywords.map((k) => (k.value ? `${k.name} ${k.value}` : k.name)).join(", "));
    if (card.owner && card.controller && card.owner !== card.controller) state.push(`owned by ${card.owner}`);
    if (state.length) tooltip.append(el("div", "tt-state", state.join(" · ")));
    tooltip.hidden = false;
    positionTooltip(e);
  }
  function positionTooltip(e) {
    if (tooltip.hidden) return;
    const pad = 14, w = tooltip.offsetWidth, h = tooltip.offsetHeight;
    let x = e.clientX + pad, y = e.clientY + pad;
    if (x + w > innerWidth - 8) x = e.clientX - w - pad;
    if (y + h > innerHeight - 8) y = Math.max(8, innerHeight - h - 8);
    tooltip.style.left = Math.max(8, x) + "px";
    tooltip.style.top = y + "px";
  }
  function hideTooltip() { tooltip.hidden = true; }

  // ── Player area ──
  function zone(label, count, cls) {
    const z = el("div", "zone " + (cls || ""));
    const l = el("div", "zone-label", label);
    if (count !== undefined) l.append(el("span", "count", String(count)));
    z.append(l);
    const r = el("div", "row");
    z.append(r);
    return [z, r];
  }

  function pileEl(name, count, cards, opts) {
    const p = el("div", "pile" + (opts.back ? " back" : ""));
    if (opts.topImage) {
      const t = el("div", "pile-top");
      t.style.backgroundImage = `url("${opts.topImage}")`;
      p.append(t);
    }
    p.append(el("div", "pile-count", String(count)));
    p.append(el("div", "pile-name", name));
    if (opts.openable) {
      p.classList.add("clickable");
      p.title = `Show ${name.toLowerCase()}`;
      p.addEventListener("click", () => openModal(opts.title || name, cards, opts.empty));
    } else {
      p.title = `${name}: ${count} card(s), contents hidden`;
    }
    return p;
  }

  function scoreEl(score, victory) {
    const s = el("div", "score");
    s.title = `Score ${score} of ${victory}`;
    for (let i = 1; i <= victory; i++) s.append(el("div", "pip" + (i <= score ? " on" : ""), String(i)));
    return s;
  }

  function poolText(pool) {
    const parts = [`<b>${pool.energy}</b> energy`];
    if (pool.unit_energy) parts.push(`<b>${pool.unit_energy}</b> unit energy`);
    for (const [d, n] of Object.entries(pool.power || {})) parts.push(`<b>${n}</b> ${d}`);
    if (pool.universal_power) parts.push(`<b>${pool.universal_power}</b> any power`);
    return parts.join(" · ");
  }

  function renderPlayer(container, pid, actionMap) {
    const b = board();
    const p = b.players[pid];
    container.replaceChildren();
    container.className = "player-area " + (container.id === "area-top" ? "top " : "bottom ") + sideClass(pid);

    // Hand
    const [hz, hr] = zone("Hand", p.hand.count, "area-hand");
    const head = el("div", "player-head");
    head.append(el("span", "player-name", playerName(pid)), scoreEl(p.score, b.turn.victory_score));
    head.append(el("span", "xp", `XP ${p.xp}`));
    if (b.turn.turn_player === pid) head.append(el("span", "badge showdown", "Turn"));
    if (b.turn.priority === pid) head.append(el("span", "badge", "Priority"));
    if (b.turn.focus === pid) head.append(el("span", "badge", "Focus"));
    hz.prepend(head);
    if (p.hand.cards) for (const c of p.hand.cards) hr.append(cardEl(c, "hand", actionMap));
    else for (let i = 0; i < p.hand.count; i++) hr.append(cardEl({ back: true, controller: pid }, "small"));
    container.append(hz);

    // Legend and champion
    const lead = el("div", "zone area-lead");
    const [lz, lr] = zone("Legend");
    lr.append(cardEl(p.legend, "", actionMap));
    const [cz, cr] = zone("Champion");
    cr.append(p.champion ? cardEl(p.champion, "", actionMap) : el("div", "slot", "played"));
    lead.append(lz, cz);
    container.append(lead);

    // Base
    const [bz, br] = zone("Base", p.base.length, "area-base");
    for (const c of p.base) br.append(cardEl(c, "", actionMap));
    markDestination(bz, { base: pid });
    container.append(bz);

    // Runes and pool
    const ready = p.runes.filter((r) => !r.exhausted).length;
    const [rz, rr] = zone("Runes", `${ready}/${p.runes.length} ready`, "area-runes");
    for (const r of p.runes) rr.append(cardEl(r, "", actionMap));
    const pool = el("div", "pool");
    pool.innerHTML = "Pool: " + poolText(p.pool);
    rz.append(pool);
    container.append(rz);

    // Piles
    const piles = el("div", "area-piles");
    const deckCards = p.main_deck.cards;
    piles.append(pileEl("Deck", p.main_deck.count, deckCards,
      { back: true, openable: !!deckCards, title: `${PLAYER_LABEL[pid]} deck (top first)` }));
    piles.append(pileEl("Rune deck", p.rune_deck.count, p.rune_deck.cards,
      { back: true, openable: !!p.rune_deck.cards, title: `${PLAYER_LABEL[pid]} rune deck` }));
    const trashTop = p.trash.length ? p.trash[p.trash.length - 1].image : null;
    piles.append(pileEl("Trash", p.trash.length, [...p.trash].reverse(),
      { openable: true, topImage: trashTop, title: `${PLAYER_LABEL[pid]} trash (newest first)` }));
    piles.append(pileEl("Banished", p.banishment.length, p.banishment,
      { openable: true, title: `${PLAYER_LABEL[pid]} banishment` }));
    container.append(piles);
  }

  // ── Battlefields ──
  function renderBattlefields(actionMap) {
    const b = board();
    const wrap = $("battlefields");
    wrap.replaceChildren();
    const top = topPlayer(), bottom = bottomPlayer();
    const mg = b.move_group;
    for (const bf of b.battlefields) {
      const box = el("div", "bf");
      if (bf.controller) box.classList.add("ctrl-" + sideClass(bf.controller));
      if (bf.card && bf.card.image) {
        const art = el("div", "bf-art");
        art.style.backgroundImage = `url("${bf.card.image}")`;
        box.append(art);
      }
      const head = el("div", "bf-head");
      const name = el("span", "bf-name", (bf.card && bf.card.name) || bf.name);
      if (bf.card) attachTooltip(name, bf.card);
      head.append(name);
      if (bf.controller) head.append(el("span", "badge ctrl-" + sideClass(bf.controller), `${PLAYER_LABEL[bf.controller]} controls`));
      else head.append(el("span", "badge", "Uncontrolled"));
      if (bf.contested) head.append(el("span", "badge contested", `Contested${bf.contested_by ? " by " + bf.contested_by : ""}`));
      if (bf.combat.in_progress || bf.combat.staged) {
        const who = bf.combat.attacker ? ` (${bf.combat.attacker} attacks)` : "";
        head.append(el("span", "badge combat", (bf.combat.in_progress ? "Combat" : "Combat pending") + who));
      }
      if (bf.showdown.in_progress || bf.showdown.staged)
        head.append(el("span", "badge showdown", bf.showdown.in_progress ? "Showdown" : "Showdown pending"));
      if (bf.is_token) head.append(el("span", "badge", "Token"));
      if (mg && mg.destination === bf.name) head.append(el("span", "badge showdown", "Move target"));
      box.append(head);

      const sideOf = (pid) => {
        const s = el("div", "bf-side " + (pid === top ? "theirs" : "mine"));
        for (const c of bf.units[pid] || []) s.append(cardEl(c, "", actionMap));
        for (const c of bf.facedown) {
          const ctrl = c.back ? c.controller : c.controller;
          if (ctrl === pid) s.append(cardEl(c, "", actionMap));
        }
        const total = (bf.units[pid] || []).reduce((n, u) => n + (u.might || 0), 0);
        if (total) s.append(el("span", "bf-might", `${PLAYER_LABEL[pid]} might ${total}`));
        return s;
      };
      box.append(sideOf(top), sideOf(bottom));
      if (bf.card) markActionable(box.querySelector(".bf-name"), bf.card, actionMap);
      markDestination(box, { battlefield: bf.id });
      wrap.append(box);
    }
  }

  // ── Side panel ──
  function renderTurnPanel() {
    const b = board(), t = b.turn;
    const p = $("turn-panel");
    p.replaceChildren(el("h2", null, "Turn"));
    const dl = el("dl", "turn-grid");
    const row = (k, v) => { dl.append(el("dt", null, k), el("dd", null, v)); };
    row("Turn", `${t.number} · ${PLAYER_LABEL[t.turn_player] || t.turn_player}`);
    row("Phase", t.phase);
    row("State", `${t.showdown ? "Showdown" : "Neutral"}, ${t.closed ? "closed" : "open"}`);
    row("Priority", t.priority ? PLAYER_LABEL[t.priority] : "—");
    if (t.focus) row("Focus", PLAYER_LABEL[t.focus]);
    if (t.passed_priority.length) row("Passed", t.passed_priority.join(", "));
    row("Score", `P1 ${b.players.P1.score} · P2 ${b.players.P2.score} (to ${t.victory_score})`);
    row("Seat", b.viewer + (frame.human_seats && frame.human_seats.length ? ` · humans: ${frame.human_seats.join(", ")}` : ""));
    p.append(dl);
    if (t.game_over) {
      const w = t.winner && t.winner !== "none" ? `${PLAYER_LABEL[t.winner]} wins` : "Game over";
      p.append(el("div", "gameover", w + (t.reason ? ` (${t.reason})` : "")));
    }
    $("turn-summary").textContent = `Turn ${t.number} · ${t.phase} · ${PLAYER_LABEL[t.turn_player] || ""}`;
  }

  function renderDecision() {
    const legal = frame.legal || [];
    const actions = $("actions");
    const prompt = $("prompt");
    actions.replaceChildren();
    const deciding = frame.type === "decision" && legal.length > 0;
    const jump = $("jump-actions");
    jump.hidden = !deciding;
    jump.textContent = `Your move (${legal.length}) ↓`;
    $("decision-title").textContent = deciding ? `Your decision (${PLAYER_LABEL[frame.player] || frame.player})` : "Actions";
    if (placing) {
      prompt.hidden = false;
      prompt.textContent = `Where should ${placing.name} go? Click a highlighted base or battlefield, ` +
        `or a button below. Esc or clicking the card again cancels.`;
    } else if (frame.prompt) { prompt.hidden = false; prompt.textContent = frame.prompt; }
    else prompt.hidden = true;

    if (!deciding) {
      let msg;
      if (board().turn.game_over) msg = "The game is over.";
      else if (frame.waiting_for) msg = `Waiting for ${PLAYER_LABEL[frame.waiting_for] || frame.waiting_for}…`;
      else msg = "Engine is thinking…";
      actions.append(el("div", "waiting", msg));
      return;
    }
    for (const a of legal) {
      const btn = el("button", "action");
      btn.dataset.index = a.index;
      const type = String(a.type || "").replace(/^P\d:\s*/, "").trim();
      if (/^(Pass|EndTurn|Done|Finish)/.test(type)) btn.classList.add("end");
      btn.append(el("span", "type", type), document.createTextNode(a.label));
      const ids = actionIds(a);
      btn.addEventListener("mouseenter", () => setHighlight(ids));
      btn.addEventListener("mouseleave", () => setHighlight([]));
      btn.addEventListener("click", () => choose(a.index));
      actions.append(btn);
    }
  }

  function setHighlight(ids) {
    highlightIds = new Set(ids);
    for (const n of document.querySelectorAll(".card.highlight")) n.classList.remove("highlight");
    if (!ids.length) return;
    // Cards carry no id attribute; re-render is cheap enough but avoid it on hover.
    for (const n of document.querySelectorAll("[data-obj]"))
      if (highlightIds.has(Number(n.dataset.obj))) n.classList.add("highlight");
  }

  function renderCost() {
    const c = board().cost, mg = board().move_group, p = $("cost-panel");
    p.replaceChildren();
    if (!c && !mg) { p.hidden = true; return; }
    p.hidden = false;
    if (c) {
      p.append(el("h2", null, "Paying a cost"));
      const line = el("div", "cost-line");
      line.innerHTML = `${c.card ? c.card.name : "Cost"}: <b>${c.energy_remaining}</b> energy, <b>${c.power_remaining}</b> power` +
        (c.power_domains.length ? ` (${c.power_domains.join("/")})` : "") + ` left · paying ${c.paying}`;
      p.append(line);
    }
    if (mg) {
      p.append(el("h2", null, "Moving units"));
      const names = mg.units.map((id) => (objIndex.get(id) || {}).name || `#${id}`);
      p.append(el("div", null, `${PLAYER_LABEL[mg.player]} → ${mg.destination}: ${names.join(", ") || "no units yet"}`));
    }
  }

  function renderChain() {
    const list = $("chain");
    list.replaceChildren();
    for (const item of board().chain) {
      const li = el("li");
      li.append(el("span", null, item.name || "(ability)"));
      li.append(el("span", "status", `${item.kind} · ${item.controller} · ${item.status}`));
      if (item.targets.length) li.append(el("div", "targets", "→ " + item.targets.map((t) => t.name).join(", ")));
      if (item.source) attachTooltip(li, item.source);
      list.append(li);
    }
  }

  function renderTextView() {
    const pre = $("text-view");
    pre.hidden = !textToggle.checked;
    if (textToggle.checked && frame) pre.textContent = frame.render_text || "";
  }

  function appendLog(m) {
    const panel = $("log-panel");
    panel.hidden = false;
    const log = $("log");
    const atBottom = log.scrollTop + log.clientHeight >= log.scrollHeight - 4;
    log.append(el("div", null, m.msg));
    while (log.childElementCount > MAX_LOG) log.firstChild.remove();
    if (atBottom) log.scrollTop = log.scrollHeight;
  }

  // ── Render ──
  function render() {
    const b = board();
    if (!b) return;
    indexObjects(b);
    const actionMap = frame.type === "decision" ? actionsByObject() : null;
    renderPlayer($("area-top"), topPlayer(), actionMap);
    renderBattlefields(actionMap);
    renderPlayer($("area-bottom"), bottomPlayer(), actionMap);
    renderTurnPanel();
    renderDecision();
    renderCost();
    renderChain();
    renderTextView();
    $("log-panel").hidden = b.viewer !== "god";
    document.title = frame.type === "decision" && (frame.legal || []).length
      ? "● Your move · Riftbound" : "Riftbound Board";
  }

  connect();
})();
