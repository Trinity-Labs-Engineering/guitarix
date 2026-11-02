# Blocks concept
Use fausst to convert the Artemis effects used by Guitarix into a browser by compiling to webassembly, and serving them with the artemis-ui-assets frontend through React in a browser.

## Informative context
General plan - RESEARCH.md

## NOTE
Feel free to radically change this fork of guitarix to achieve this goal.

# Milestone 0: Bringup
The nam-a2-support branch of Guitarix has had at least one push since this branch was created - please bring in the new effects added by these pushes

# Milestone 1: Blocks Proof Of Concept
Examine each effect in the newly forked Guitarix repo for their portable faust feasibility, choosing only the easiest subset to convert first.
Then, convert / compile each such effect to run from a new browser app (placed in the blocks repo), such that I can test each effect one at a time through the new app, using UI from the attached artemis-ui-assets repo (where available).
To test this, I will use my audio interface to run the effects by routing the audio to the app through Jack.

# Milestone 2: Complete candidate catalogue
Compile and expose every Candidate source in `blocks/docs/EFFECT_INVENTORY.md`,
including LV2 copies, stereo processors, generators and helper blocks. The checkout
contains 361 Candidate rows, rather than the request's 341; preserve that baseline
in `blocks/scripts/milestone2-candidates.json`. Categorise the collection by effect
type and use matching artwork from the updated `artemis-ui-assets` submodule.
Retain the one-effect audition workflow, existing audio setup and output protection.
Record compatibility changes and validate every compiled processor and its UI.

# Goal
Implement Milestones 1 and 2 end to end on this machine