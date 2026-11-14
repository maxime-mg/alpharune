# ─── Play app: Riftbound in the browser ──────────────────────────────────────
# Everything here sits on top of the engine (riftbound_core) and nothing in
# src/ includes it. Turn it off with -DRIFTBOUND_BUILD_PLAY=OFF.
#
#   riftbound_play_lib    HTTP + WebSocket server, HumanAgent, per-seat board view
#   riftbound_play        the program: one interactive game
#   riftbound_play_tests  tests for the play layer
#   web/                  the board page, served from disk (see --web-root)

add_library(riftbound_play_lib STATIC
    src/board_view.cpp
    src/classic_page.cpp
    src/play_server.cpp
)
target_include_directories(riftbound_play_lib PUBLIC src)
target_link_libraries(riftbound_play_lib PUBLIC riftbound_core Boost::system)

# riftbound_play needs the OpenSpiel game TUs compiled straight into the
# executable for MCTS seats (see the riftbound target in the root file).
add_executable(riftbound_play
    src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/agents/mcts_agent.cpp
    ${PROJECT_SOURCE_DIR}/src/openspiel/riftbound_game.cpp
    ${PROJECT_SOURCE_DIR}/src/openspiel/riftbound_state.cpp
    $<TARGET_OBJECTS:algorithms>
    $<TARGET_OBJECTS:game_transforms>
    $<TARGET_OBJECTS:utils>
)
target_compile_definitions(riftbound_play PRIVATE
    RIFTBOUND_PLAY_WEB_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/web")
target_include_directories(riftbound_play PRIVATE ${open_spiel_SOURCE_DIR})
target_link_libraries(riftbound_play PRIVATE
    riftbound_play_lib
    open_spiel_core
    Boost::program_options
    Boost::system
    Boost::filesystem
    absl::algorithm absl::flags absl::flags_parse absl::flat_hash_map
    absl::optional absl::random_random absl::str_format absl::strings absl::time
)

file(GLOB RIFTBOUND_PLAY_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
add_executable(riftbound_play_tests ${RIFTBOUND_PLAY_TEST_SOURCES})
# The engine repo root, for tests/cards/card_test_fixture.h.
target_include_directories(riftbound_play_tests PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(riftbound_play_tests PRIVATE riftbound_play_lib GTest::gtest_main)
gtest_discover_tests(riftbound_play_tests)
