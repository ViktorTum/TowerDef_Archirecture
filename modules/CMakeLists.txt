# Layers are listed bottom-up. A module may depend only on modules above it in
# this list (see docs/ARCHITECTURE.md). Layers that are not written yet are
# added here in the commit that introduces them.

add_subdirectory(core)          # A  td::core
# add_subdirectory(defs)        # B  td::defs
# add_subdirectory(validation)  # C  td::validation
# add_subdirectory(persistence) # D  td::persistence
# add_subdirectory(mechanics)   # E  td::mechanics
# add_subdirectory(spatial)     # F  td::spatial
# add_subdirectory(state)       # G  td::state
# add_subdirectory(commands)    # H  td::commands
# add_subdirectory(engine)      # I  td::engine
# add_subdirectory(campaign)    # J  td::campaign
# add_subdirectory(replay)      # K  td::replay
# add_subdirectory(editor)      # L  td::editor
# add_subdirectory(api)         # M  td::api / td::godot
