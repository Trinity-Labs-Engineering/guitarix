# Blocks concept
Use fausst to convert the Artemis effects used by Guitarix into a browser by compiling to webassembly, and serving them with the artemis-ui-assets frontend through React in a browser.

# Milestone 0: Bringup
The nam-a2-support branch of Guitarix has had at least one push since this branch was created - please bring in the new effects added by these pushes

# Milestone 1: Blocks Proof Of Concept
Examine each effect in the newly forked Guitarix repo for their portable faust feasibility, choosing only the easiest subset to convert first.
Then, convert / compile each such effect to run from a new browser app (placed in blocks - move the artemis-ui-assets submodule here first)