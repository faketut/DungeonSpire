# Checklist for Demo Submission

**Note:** EACH DEMO WILL RUN FOR NO MORE THAN 30 MINUTES. TO GET A CHANCE TO SHOW CASE ALL FEATURES YOU MUST BE ABLE TO LOAD FILES THROUGH A COMMAND LINE ARGUMENT AS SPECIFIED IN THE ASSIGNMENT. FAILURE TO DO SO WILL RESULT IN DELAYS IN MARKING YOUR SUBMISSION DURING THE DEMO. NO EXTRA TIME WILL BE GIVEN.

---

## Total: __________/62

## Bonus: __________/10

## General: __/10

- [x] Program includes a Makefile
- [x] Program compiles and runs
- [x] Program displays initial board
- [x] Board displays current floor
- [x] Board displays current money
- [x] Current level and score are updated correctly
- [x] Player is asked to choose a race
- [x] Each race can be selected (randomly check 2-3)
- [x] Each race has appropriate starting statistics (for the ones selected above)
- [ ] PC and stairs never spawn in same room

## Movement: __/7

- [x] Can move in all directions
- [x] Can move diagonally around corners
- [x] Cannot move outside the bounds of a chamber
- [x] Enemies move whenever a valid command is issued
- [x] Cannot move outside the bounds of a passage
- [x] Enemies, the PC, Gold, and Potions can never occupy the same floor tile.
    The only exception is the PC can walk over a tile with a dragon hoard as long as the associated dragon is alive, but PC still cannot pick up the gold until the dragon is dead.
- [x] Stairs move PC to new floor (either newly generated or from the provided file)

## Random Generation: __/9
(Program is run with no argument supplied for the load file)
(A lot of this will require actually scanning through the relevant code)

- [x] All enemies can be generated
- [x] 10 potions are generated per floor
- [x] Potion generation follows prescribed distribution (may need to check code)
- [x] 10 treasure piles are generated per floor
- [x] Gold generation follows prescribed distribution (may need to check code)
- [x] 20 enemies (does not count Dragons) are generated per floor
- [x] Enemy generation follows prescribed distribution (may need to check code)
- [x] Order of generation occurs as set out in design specification (may need to check code)
- [x] Enemies, Gold, and Potions only spawn on floor tiles

## Command Line Argument: __/3

- [x] Program can load a file specified as a command line argument.(FILE: cc3kfloor.txt)

## Potions: __/9

- [x] Can use Potion in any of the cardinal directions (FILE: potions.txt)
- [x] Each potion has the correct effect on the PC (0.5/potion)(FILE: potions.txt)
- [x] Achieving an HP of 0 causes player to lose game via potions(FILE: death.txt)
- [x] Atk/Def cannot drop below 0 (FILE: lowattack.txt and FILE: lowdefence.txt)
- [x] Potions are unknown until they are used once
- [x] Potion effects (for Atk and Def) disappear when transitioning levels
- [x] Elves get positive benefits from negative potions

## Combat: __/12

- [x] PC can attack in any direction
- [x] PC can be attacked by more than one enemy at once.
- [x] Achieving an HP of 0 causes player to lose game via combat (FILE: attack_all.txt)
- [x] Merchants should not attack the PC initially (FILE: merchant.txt)
- [x] After slaying a MERCHANT all other merchants should become hostile (both in this level (1pt) and further levels (1pt)) (FILE: merchant.txt)
- [x] Enemies, except MERCHANT and DRAGON, are hostile to the PC and attack whenever possible
- [x] DRAGON is hostile when PC is next to DRAGON Horde (FILE: dragon.txt)
- [x] Damage is correctly calculated to the PC (May need to check code)
- [x] Damage is correctly calculated to Enemies (may need to check code to double check enemies)
- [x] Ensure that Enemies have a 50% chance not to hit PC (may need to check source)
- [x] PC never misses a valid attack. (may need to check source - but should be able to determine while playing) Note: Valid means an enemy is there

## Gold: __/5

- [x] Gold is picked up when the PC walks on it (except DRAGON Horde if the DRAGON is alive). Can be picked up in any direction (FILE: gold.txt)
- [x] DRAGON Horde cannot be picked up until DRAGON is slain (1 before, 1 after)
- [x] Slaying a MERCHANT drops a MERCHANT Horde (check score after slaying a merchant) (Dropped merchant horde can be automatically added or require the player to walk over it, both are valid)
- [x] Dwarves get double gold/orcs get half gold/humans get 50% bonus at the end of the game. (Check one or two of these cases randomly)

## Ending the Game: __/7

- [x] Quit works
- [x] Restart works (should prompt for a new race)
- [x] If the player restarts, or the PC dies, or reaches the end of floor 5 then the game should prompt the player to quit or select a race. Note: Prompting for a new race by itself is fine.
- [x] Program leaks only a modest amount of memory in text mode (apart from libraries)
- [x] Program leaks no memory in text mode (apart from libraries)
- [x] No segfaults/crashes/errors throughout entire demo (except for accidental invalid input)
  - -1 mark for each segfault/crash/error (except for accidental invalid input) up to a maximum of -5

## Bonus: (Note to marker: Use the printed page containing the bonus.txt document that the group submitted. If the bonus.txt file was not submitted, but the group claims to have implemented the bonus, try your best to describe the bonus feature below please be descriptive and indicate level of difficulty. Difficulty numbers should be filled once the group has left to avoid arguments. 1 - easy and 10 - very difficult):