/* D&D 5E Character Generator
 *
 * Overview:
 *
 * This pseudocode is intended to demonstrate the general flow of the program.
 *
 * set default variables for character stats
 * have player choose race, save racial traits into memory.
 * if necessary, choose subrace
 * have player choose class
 * determine method of stat generation (standard array, roll, point buy)
 * if standard array, present array values as constexpr variables.
 * if roll, generate 4d6 rolls - lowest roll for each value
 * if point buy, use point-buy algorithm and stat limits
 * once stats are determined, sum the stats and racial traits, save into memory
 * (to be continued)
 * */

#include <iostream>
#include <cmath>
#include <cctype>
#include <ctime>
#include <algorithm>
#include <random>

using namespace std;

string current_version = "v0.0.12";

//stat_roll initial values
int r1, r2, r3, r4 = 0;
int s1, s2, s3, s4, s5, s6 = 0;
int d2, d4, d6, d8, d10, d12, d20, d100 = 0;


//Standard array values
constexpr int array_value_15 = 15;
constexpr int array_value_14 = 14;
constexpr int array_value_13 = 13;
constexpr int array_value_12 = 12;
constexpr int array_value_10 = 10;
constexpr int array_value_8 = 8;

//Character class initialization
string player_class = "";
string player_hitdie = "d100";

//Character array variables
int charstat_1 = 0;
int charstat_2 = 0;
int charstat_3 = 0;
int charstat_4 = 0;
int charstat_5 = 0;
int charstat_6 = 0;

//Future input-limiting integers; not currently used.
constexpr int attribute_max = 20;
constexpr int attribute_min = 3;

//Variable to check if input is an integer; not currently used
int is_integer = 0;

//Character attribute variables initial values
int STR = 10;
int DEX = 10;
int CON = 10;
int WIS = 10;
int INT = 10;
int CHA = 10;

//Character attribute modifier variables initial values
int STR_mod = 0;
int DEX_mod = 0;
int CON_mod = 0;
int WIS_mod = 0;
int INT_mod = 0;
int CHA_mod = 0;

//Variable for presence/absence of '+/-' sign
string mod_prefix = "";

//Racial Modifier Integers
signed int race_mod_1 = -2;
signed int race_mod_2 = -1;
signed int race_mod_3 = +1;
signed int race_mod_4 = +2;

void input_delay() {
    cout << "Press any key to continue.\n";
    cin.get();
}

//Race Selection Variables
char race = 'z';
char response;
string player_race = "";

//Subrace Selection Variables
char subrace_dwarf = 'z';
char subrace_elf = 'z';
char subrace_halfling = 'z';
char subrace_human = 'z';
char subrace_dragonborn = 'z';
char subrace_dragonborn_chromatic = 'z';
char subrace_dragonborn_metallic = 'z';
char subrace_gnome = 'z';
char subrace_halfelf = 'z';
char subrace_halforc = 'z';
char subrace_tiefling = 'z';
string player_subrace = "";

//Dice rolling algorithms (random)

int d100_roll(int d100) {
    d100 = (rand() % (100)) + 1;
    return d100;
}

int d20_roll(int d20) {
    d20 = (rand() % (20)) + 1;
    return d20;
}

int d12_roll(int d12) {
    d12 = (rand() % (12)) + 1;
    return d12;
}

int d10_roll(int d10) {
    d10 = (rand() % (10)) + 1;
    return d10;
}

int d8_roll(int d8) {
    d8 = (rand() % (8)) + 1;
    return d8;
}

int d6_roll(int d6) {
    d6 = (rand() % (6)) + 1;
    return d6;
}

int d4_roll(int d4) {
    d4 = (rand() % (4)) + 1;
    return d4;
}

int d2_roll(int d2) {
    d2 = (rand() % (2)) + 1;
    return d2;
}

void manual_stat_assign() {
    /*if standard_array method chosen, load standard_array values into array
     * else if stat_roll method chosen, load stat_roll values into array
     * else if point_buy method chosen, goto point_buy()
     * create temporary copy of stat array values
     * label each subroutine to allow loop
     * sequentially ask player to assign array values to stats
     * confirm each assignation after selection
     * if confirmation != true, put value back and go back to subroutine label
     * else if confirmation == true, remove used values from selection
     * create final stats label
     * at end of stat assignation, present all stats, ask for final confirmation
     * if final confirmation == true, finalize stat values and delete temporary stat array copy, goto racial_modifiers()
     * else if final confirmation != true, ask for further confirmation to start over
     * if further confirmation == true, clear all stat values, load stat array values from temporary copy, loop back to first stat assignation
     * else if further confirmation != true, loop to final stats label
     */
}

void point_buy() {

}

void metallic_dragonborn_choice() {
    cin.get();
    input_delay();
    METALLIC_ANCESTRY_SELECT:
    cout << "Choose your Metallic Ancestry.\n\n";
    cout << "a.  Brass\nb.  Bronze\nc.  Copper\nd.  Gold\ne.  Silver\n";
    cin.get(subrace_dragonborn_metallic);
    switch (subrace_dragonborn_metallic) {
        case 'a' :
            cout << "You chose to be a Brass Dragon.  Brass dragons are know to be very chatty and love the heat of deserts.  They can breathe fire in a 5 by 30 ft. line.\n";
            cout << "Are you sure you want to be a Brass Dragon?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto METALLIC_ANCESTRY_SELECT;
            else if (response == 'y')
                player_subrace = "Brass Dragon";
            cout << "You chose to be a Brass Dragon";
            break;
        case 'b' :
            cout << "You chose to be a Bronze Dragon.  Bronze dragons are inquisitive and fascinated by warfare.  They can breathe lightning in a 5 by 30 ft. line.\n";
            cout << "Are you sure you want to be a Bronze Dragon?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto METALLIC_ANCESTRY_SELECT;
            else if (response == 'y')
                player_subrace = "Bronze Dragon";
            cout << "You chose to be a Bronze Dragon";
            break;
        case 'c' :
            cout << "You chose to be a Copper Dragon.  Copper dragons are born pranksters and enjoy rocky mountains.  They can breathe acid in a 5 by 30 ft. line.\n";
            cout << "Are you sure you want to be a Copper Dragon?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto METALLIC_ANCESTRY_SELECT;
            else if (response == 'y')
                player_subrace = "Copper Dragon";
            cout << "You chose to be a Copper Dragon";
            break;
        case 'd' :
            cout << "You chose to be a Gold Dragon.  Gold dragons are known to be knowledgable and reclusive.  They can breathe fire in a 15 ft. cone.\n";
            cout << "Are you sure you want to be a Gold Dragon?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto METALLIC_ANCESTRY_SELECT;
            else if (response == 'y')
                player_subrace = "Gold Dragon";
            cout << "You chose to be a Gold Dragon";
            break;
        case 'e' :
            cout << "You chose to be a Silver Dragon.  Silver dragons are regal and often disguise themselves as humans.  They can breathe cold in a 15 ft. cone.\n";
            cout << "Are you sure you want to be a Silver Dragon?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto METALLIC_ANCESTRY_SELECT;
            else if (response == 'y')
                player_subrace = "Silver Dragon";
            cout << "You chose to be a Silver Dragon";
            break;
        default :
            cout << "That is not a valid choice.\n\n";
            goto METALLIC_ANCESTRY_SELECT;
    }
}

void chromatic_dragonborn_choice() {
    cin.get();
    input_delay();
    CHROMATIC_ANCESTRY_SELECT:
    cout << "Choose your Chromatic Ancestry.\n\n";
    cout << "a.  Black\nb.  Blue\nc.  Green\nd.  Red\ne.  White\n";
    cin.get(subrace_dragonborn_chromatic);
    switch (subrace_dragonborn_chromatic) {
        case 'a' :
            cout << "You chose to be a Black Dragon.  Black dragons are vile tempered and inclined to cruelty.  They can breath acid in a 5 by 30 ft. line.\n";
            cout << "Are you sure you want to be a Black Dragon?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto CHROMATIC_ANCESTRY_SELECT;
            else if (response == 'y')
                player_subrace = "Black Dragon";
            cout << "You chose to be a Black Dragon";
            break;
        case 'b' :
            cout << "You chose to be a Blue Dragon.  Blue dragons are manipulative and are partial to arid wastelands.  They can breath lightning in a 5 by 30 ft. line.\n";
            cout << "Are you sure you want to be a Blue Dragon?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto CHROMATIC_ANCESTRY_SELECT;
            else if (response == 'y')
                player_subrace = "Blue Dragon";
            cout << "You chose to be a Blue Dragon";
            break;
        case 'c' :
            cout << "You chose to be a Green Dragon.  Green dragons are territorial and live in forests.  They can breath poison in a 15 ft. line.\n";
            cout << "Are you sure you want to be a Green Dragon?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto CHROMATIC_ANCESTRY_SELECT;
            else if (response == 'y')
                player_subrace = "Green Dragon";
            cout << "You chose to be a Green Dragon";
            break;
        case 'd' :
            cout << "You chose to be a Red Dragon.  Red dragons are greedy and vain.  They can breath fire in a 15 ft. line.\n";
            cout << "Are you sure you want to be a Red Dragon?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto CHROMATIC_ANCESTRY_SELECT;
            else if (response == 'y')
                player_subrace = "Red Dragon";
            cout << "You chose to be a Red Dragon";
            break;
        case 'e' :
            cout << "You chose to be a White Dragon.  White dragons are bestial and have violent tendencies.  They can breath cold in a 15 ft. cone.\n";
            cout << "Are you sure you want to be a White Dragon?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto CHROMATIC_ANCESTRY_SELECT;
            else if (response == 'y')
                player_subrace = "White Dragon";
            cout << "You chose to be a White Dragon";
            break;
        default:
            cout << "That is not a valid choice.\n\n";
            goto CHROMATIC_ANCESTRY_SELECT;
    }
}

string dwarf_subrace() {
    cin.get();
    input_delay();
    DWARF_SUBRACE_SELECT:
    cout << "What subrace of Dwarf will you be?\n\n";
    cout << "a.  Hill Dwarf\nb.  Mountain Dwarf\nc.  Duergar\n";
    cin.get(subrace_dwarf);
    switch (subrace_dwarf) {
        case 'a' :
            cout << "You chose to be a Hill Dwarf.  Hill Dwarves get +1 to WIS and increase their HP maximum by 1, and does so every time you gain a level.\n\n";
            cout << "Are you sure you want to be a Hill Dwarf?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto DWARF_SUBRACE_SELECT;
            else if (response == 'y')
                player_subrace = "Hill Dwarf";
            cout << "You chose to be a Hill Dwarf";
            return player_subrace;
            break;
        case 'b' :
            cout << "You chose to be a Mountain Dwarf.  Mountain Dwarves get +2 to STR and have proficiency with Light and Medium Armor.\n\n";
            cout << "Are you sure you want to be a Mountain Dwarf?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto DWARF_SUBRACE_SELECT;
            else if (response == 'y')
                player_subrace = "Mountain Dwarf";
            cout << "You chose to be a Mountain Dwarf";
            break;
        case 'c' :
            cout << "You chose to be a Duergar.  Duergar get +1 to STR and have advantage on saving throws against illusions and against being charmed or paralyzed.\n\n";
            cout << "Are you sure you want to be a Duergar?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto DWARF_SUBRACE_SELECT;
            else if (response == 'y')
                player_subrace = "Duergar";
            break;
        default:
            cout << "That is not a valid choice.\n\n";
            goto DWARF_SUBRACE_SELECT;

    }
}

void elf_subrace() {
    cin.get();
    input_delay();
    ELF_SUBRACE_SELECT:
    cout << "What subrace of Elf will you be?\n\n";
    cout << "a.  High Elf\nb.  Wood Elf\nc.  Drow\n";
    cin.get(subrace_elf);
    switch (subrace_elf) {
        case 'a' :
            cout << "You chose to be a High Elf.\n\n";
            cout << "Are you sure you want to be a High Elf?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto ELF_SUBRACE_SELECT;
            else if (response == 'y')
                player_subrace = "High Elf";
            cout << "You chose to be a High Elf";
            break;
        case 'b' :
            cout << "You chose to be a Wood Elf.\n\n";
            cout << "Are you sure you want to be a Wood Elf?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto ELF_SUBRACE_SELECT;
            else if (response == 'y')
                player_subrace = "Wood Elf";
            cout << "You chose to be a Wood Elf";
            break;
        case 'c' :
            cout << "You chose to be a Drow.\n\n";
            cout << "Are you sure you want to be a Drow?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto ELF_SUBRACE_SELECT;
            else if (response == 'y')
                player_subrace = "Drow";
            cout << "You chose to be a Drow";
            break;
        default:
            cout << "That is not a valid choice.\n\n";
            goto ELF_SUBRACE_SELECT;

    }
}

void halfling_subrace() {
    cin.get();
    input_delay();
    HALFLING_SUBRACE_SELECT:
    cout << "What subrace of Halfling will you be?\n\n";
    cout << "a.  Lightfoot\nb.  Stout\n";
    cin.get(subrace_halfling);
    switch (subrace_halfling) {
        case 'a' :
            cout << "You chose to be a Lightfoot Halfling.  Lightfoots get +1 to CHA and are naturally stealthy.\n\n";
            cout << "Are you sure you want to be a Lightfoot Halfling?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto HALFLING_SUBRACE_SELECT;
            else if (response == 'y')
                player_subrace = "Lightfoot Halfling";
            cout << "You chose to be a Lightfoot Halfling";
            break;
        case 'b' :
            cout << "You chose to be a Stout Halfling.  Stouts get +1 to CON and are resistant to poison.\n\n";
            cout << "Are you sure you want to be a Stout Halfling?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto HALFLING_SUBRACE_SELECT;
            else if (response == 'y')
                player_subrace = "Stout Halfling";
            cout << "You chose to be a Stout Halfling";
            break;
        default :
            cout << "That is not a valid choice.\n\n";
            goto HALFLING_SUBRACE_SELECT;

    }
}

void dragonborn_subrace() {
    cin.get();
    input_delay();
    DRAGONBORN_SUBRACE_SELECT:
    cout << "DESCRIPTIVE TEXT.\n\n";
    cout << "a.  Metallic Ancestry\n\nb.  Chromatic Ancestry\n\n";
    cin.get(subrace_dragonborn);
    switch (subrace_dragonborn) {
        case 'a' :
            cout << "You chose a Metallic Ancestry.  Metallic dragons tend towards altruism, law and order.\n\n";
            cout << "Are you sure you want to be a Metallic Dragonborn?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto DRAGONBORN_SUBRACE_SELECT;
            else if (response == 'y')
                metallic_dragonborn_choice();
            break;
        case 'b' :
            cout
                    << "You chose a Chromatic Ancestry.  Chromatic dragons are generally greedy, predatory, and chaotic.\n\n";
            cout << "Are you sure you want to be a Chromatic Dragonborn?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto DRAGONBORN_SUBRACE_SELECT;
            else if (response == 'y')
                chromatic_dragonborn_choice();
            break;
        default :
            cout << "That is not a valid choice.\n\n";
            goto DRAGONBORN_SUBRACE_SELECT;
    }

}

void gnome_subrace() {
    cin.get();
    input_delay();
    GNOME_SUBRACE_SELECT:
    cout << "What subrace of Gnome will you be?\n\n";
    cout << "a.  Forest Gnome\nb.  Rock Gnome\nc.  Svirfneblin (Deep Gnome)\n";
    cin.get(subrace_gnome);
    switch (subrace_gnome) {
        case 'a' :
            cout << "You chose to be a Forest Gnome.  Forest Gnomes get +1 to DEX and know the Minor Illusion cantrip.\n\n";
            cout << "Are you sure you want to be a Forest Gnome?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto GNOME_SUBRACE_SELECT;
            else if (response == 'y')
                player_subrace = "Forest Gnome";
            break;
        case 'b' :
            cout << "You chose to be a Rock Gnome.  Rock Gnomes get +1 to CON and have proficiency with artisan's tools.\n\n";
            cout << "Are you sure you want to be a Rock Gnome?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto GNOME_SUBRACE_SELECT;
            else if (response == 'y')
                player_subrace = "Rock Gnome";
            break;
        case 'c' :
            cout << "You chose to be a Deep Gnome.  Deep Gnomes get +1 to DEX and have advantage on Stealth checks to hide in rocky terrain.\n\n";
            cout << "Are you sure you want to be a Deep Gnome?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto GNOME_SUBRACE_SELECT;
            else if (response == 'y')
                player_subrace = "Deep Gnome";
            break;
        default :
            cout << "That is not a valid choice.\n\n";
            goto GNOME_SUBRACE_SELECT;

    }
}

void halfelf_subrace() {

}

void halforc_subrace() {

}

void tiefling_subrace() {

}

void choose_race() {
    cout << "Welcome to the 5th Edition D&D Character Creation Tool!  Current revision is:  " << current_version
         << "\n\n";
    input_delay();
    cout << "Here you will create your very own 5E D&D Character.  You'll start out by choosing your Character's Race.\n";
    cout << "Each race has different attributes, both good and bad.  Don't worry if you choose a race and change your mind; you will be asked to confirm your selection before anything is finalized.\n\n";
    input_delay();
    Choose_Race:
    cout << "Choose a Race for your Character.  Type the letter of the race you'd like to examine:\n\n";
    cout << "a.  Dwarf\nb.  Elf\nc.  Halfling\nd.  Human\ne.  Dragonborn\nf.  Gnome\ng.  Half-Elf\nh.  Half-Orc\ni.  Tiefling\n";
    cin.get(race);
    switch (race) {
        case 'a' :
            cout << "You chose to be a Dwarf.  Dwarves are tough and resistant to poison, but are slower than full-sized races.\n\n";
            cout << "Are you sure you want to be a Dwarf?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto Choose_Race;
            else if (response == 'y')
                player_race = "Dwarf";
            cout << "You chose to be a Dwarf.\n";
            dwarf_subrace();
            break;
        case 'b' :
            cout << "You chose to be an Elf.  Elves are agile and excellent archers, but are less hardy than other races.\n\n";
            cout << "Are you sure you want to be an Elf?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto Choose_Race;
            else if (response == 'y')
                player_race = "Elf";
            cout << "You chose to be an Elf.\n";
            elf_subrace();
            break;
        case 'c' :
            cout << "You chose to be a Halfling.  Halflings are a small but clever race, but are generally not suited to hand-to-hand combat.\n\n";
            cout << "Are you sure you want to be a Halfling?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto Choose_Race;
            else if (response == 'y')
                player_race = "Halfling";
            cout << "You chose to be a Halfling.\n";
            halfling_subrace();
            break;
        case 'd' :
            cout << "You chose to be a Human.  Humans are the most versatile of all races; capable of doing well in all things, but rarely mastering any. \n\n";
            cout << "Are you sure you want to be a Human?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto Choose_Race;
            else if (response == 'y')
                player_race = "Human";
            cout << "You chose to be a Human.\n";
            break;
        case 'e' :
            cout << "You chose to be a Dragonborn.  Dragonborn are humanoid dragon-like beings, powerful and magically gifted.\n\n";
            cout << "Are you sure you want to be a Dragonborn?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto Choose_Race;
            else if (response == 'y')
                player_race = "Dragonborn";
            cout << "You chose to be a Dragonborn.\n";
            dragonborn_subrace();
            break;
        case 'f' :
            cout << "You chose to be a Gnome.  Gnomes are small and dexterous, with a strong affinity for magic, though physically weaker than most other races.\n\n";
            cout << "Are you sure you want to be a Gnome?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto Choose_Race;
            else if (response == 'y')
                player_race = "Gnome";
            cout << "You chose to be a Gnome.\n";
            gnome_subrace();
            break;
        case 'g' :
            cout << "You chose to be a Half-Elf.  Half-Elves are the best parts of their parents; capable of elvish skill but with the ingenuity and strength of humans.\n\n";
            cout << "Are you sure you want to be a Half-Elf?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto Choose_Race;
            else if (response == 'y')
                player_race = "Half-Elf";
            cout << "You chose to be a Half-Elf.\n";
            halfelf_subrace();
            break;
        case 'h' :
            cout << "You chose to be a Half-Orc.  Smarter than full Orcs, stronger than Humans, Half-Orcs are, fairly or not, viewed with greater suspicion than other, more comely races.\n\n";
            cout << "Are you sure you want to be a Half-Orc?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto Choose_Race;
            else if (response == 'y')
                player_race = "Half-Orc";
            cout << "You chose to be a Half-Orc.\n";
            halforc_subrace();
            break;
        case 'i' :
            cout << "You chose to be a Tiefling.  Tieflings are descendants of the pairing of a devil and a human.  They are skilled in magic and physically gifted, but are often chaotic in nature.\n\n";
            cout << "Are you sure you want to be a Tiefling?\n\n";
            cout << "Type y for YES or n for NO.\n\n";
            cin >> response;
            if (response == 'n')
                goto Choose_Race;
            else if (response == 'y')
                player_race = "Tiefling";
            cout << "You chose to be a Tiefling.\n";
            break;
        default:
            cout << "That is not a valid choice.\n\n";
            goto Choose_Race;
    }
}

void set_STR() {
    STR_Input:
    cout << "Please enter a STR value from 1 to 30.  Default is 10.\n";
    cin >> STR;
    if (STR < 1) {
        cout << "That is not a valid input.  Please enter a value higher than 0.\n";
        STR = 10;
        goto STR_Input;
    } else if (STR > 30) {
        cout << "That is not a valid input. Please enter a value lower than 31.\n";
        STR = 10;
        goto STR_Input;
    } else
        STR_mod = int(STR / 2) - 5;
    if (STR > 11)
        mod_prefix = "+";
    cout << "Your STR is " << STR << " and its modifier is " << mod_prefix << STR_mod << ".\n";
    mod_prefix = "";
}

void set_DEX() {
    DEX_Input:
    cout << "Please enter a DEX value from 1 to 30.  Default is 10.\n";
    cin >> DEX;
    if (DEX < 1) {
        cout << "That is not a valid input.  Please enter a value higher than 0.\n";
        DEX = 10;
        goto DEX_Input;
    } else if (DEX > 30) {
        cout << "That is not a valid input.  Please enter a value lower than 31.\n";
        DEX = 10;
        goto DEX_Input;
    } else
        DEX_mod = int(DEX / 2) - 5;
    if (DEX > 11)
        mod_prefix = "+";
    cout << "Your DEX is " << DEX << " and its modifier is " << mod_prefix << DEX_mod << ".\n";
    mod_prefix = "";
}

void set_CON() {
    CON_Input:
    cout << "Please enter a CON value from 1 to 30.  Default is 10.\n";
    cin >> CON;
    if (CON < 1) {
        cout << "That is not a valid input.  Please enter a value higher than 0.\n";
        CON = 10;
        goto CON_Input;
    } else if (CON > 30) {
        cout << "That is not a valid input.  Please enter a value lower than 31.\n";
        CON = 10;
        goto CON_Input;
    } else
        CON_mod = int(CON / 2) - 5;
    if (CON > 11)
        mod_prefix = "+";
    cout << "Your CON is " << CON << " and its modifier is " << mod_prefix << CON_mod << ".\n";
    mod_prefix = "";
}

void set_INT() {
    INT_Input:
    cout << "Please enter a INT value from 1 to 30.  Default is 10.\n";
    cin >> INT;
    if (INT < 1) {
        cout << "That is not a valid input.  Please enter a value higher than 0.\n";
        INT = 10;
        goto INT_Input;
    } else if (INT > 30) {
        cout << "That is not a valid input.  Please enter a value lower than 31.\n";
        INT = 10;
        goto INT_Input;
    } else
        INT_mod = int(INT / 2) - 5;
    if (INT > 11)
        mod_prefix = "+";
    cout << "Your INT is " << INT << " and its modifier is " << mod_prefix << INT_mod << ".\n";
    mod_prefix = "";
}

void set_WIS() {
    WIS_Input:
    cout << "Please enter a WIS value from 1 to 30.  Default is 10.\n";
    cin >> WIS;
    if (WIS < 1) {
        cout << "That is not a valid input.  Please enter a value higher than 0.\n";
        WIS = 10;
        goto WIS_Input;
    } else if (WIS > 30) {
        cout << "That is not a valid input.  Please enter a value lower than 31.\n";
        WIS = 10;
        goto WIS_Input;
    } else
        WIS_mod = int(WIS / 2) - 5;
    if (WIS > 11)
        mod_prefix = "+";
    cout << "Your WIS is " << WIS << " and its modifier is " << mod_prefix << WIS_mod << ".\n";
    mod_prefix = "";
}

void set_CHA() {
    CHA_Input:
    cout << "Please enter a CHA value from 1 to 30.  Default is 10.\n";
    cin >> CHA;
    if (CHA < 1) {
        cout << "That is not a valid input.  Please enter a value higher than 0.\n";
        CHA = 10;
        goto CHA_Input;
    } else if (CHA > 30) {
        cout << "That is not a valid input.  Please enter a value lower than 31.\n";
        CHA = 10;
        goto CHA_Input;
    } else
        CHA_mod = int(CHA / 2) - 5;
    if (CHA > 11)
        mod_prefix = "+";
    cout << "Your CHA is " << CHA << " and its modifier is " << mod_prefix << CHA_mod << ".\n";
    mod_prefix = "";
}

int race_dwarf() {
    CON = CON +2;
    if (player_subrace == "Hill Dwarf"){
        WIS = WIS + 1;
    }
    else if (player_subrace == "Mountain Dwarf"){
        STR = STR + 2;
    }
    else if (player_subrace == "Duergar"){
        STR = STR + 1;
    }
    else cout << "You should not see this message\n\n";
    return 1001;
    //Jump to next section//
}

string race_elf() {
    DEX = DEX + 2;
    if (player_subrace == "High Elf"){
        INT = INT + 1;
    }
    else if(player_subrace == "Wood Elf"){
        WIS = WIS + 1;
    }
    else if(player_subrace == "Drow"){
        CHA = CHA + 1;
    }
}

string race_halfing() {
    DEX = DEX + 2;
    if(player_subrace == "Lightfoot Halfling"){
        CHA = CHA + 1;
        //Jump to next section//
    }
    else if(player_subrace == "Stout Halfling") {
        CON = CON + 1;
        //Jump to next section//
    }

}

string race_human() {
    STR = STR + 1;
    DEX = DEX + 1;
    CON = CON + 1;
    INT = INT + 1;
    WIS = WIS + 1;
    CHA = CHA + 1;
    //Jump to next section//
}

string race_dragonborn() {
    STR = STR + 2;
    CHA = CHA + 1;
    //Fill out case for draconic_ancestry; set breath weapon//
    //Jump to next section//
}

string race_gnome() {
    INT = 1;
    if(player_subrace == "Forest Gnome") {
        DEX = DEX + 1;
        //Jump to next section//
    } else if(player_subrace == "Rock Gnome") {
        CON = CON + 1;
        //Jump to next section//
    }
}

string race_halfelf() {
    CHA = CHA + 2;
    //Implement Wood Elf, etc., ancestry calls//
    //Implement halfelf_bonus() function//
}

string race_halforc() {

}

string race_tiefling() {

}

int class_barbarian(){


}

int class_bard(){

}

int class_cleric(){

}

int class_druid(){

}

int class_fighter(){

}

int class_monk(){

}

int class_paladin(){

}

int class_ranger(){

}

int class_rogue(){

}

int class_sorcerer(){

}

int class_warlock(){

}

int class_wizard(){

}

void char_attributes() {
    set_STR();
    set_DEX();
    set_CON();
    set_INT();
    set_WIS();
    set_CHA();
}

//Roll 4d6 and discard lowest value for random stat generation
int stat_roll(int final_total){
    r1 = d6_roll(1);
    r2 = d6_roll(2);
    r3 = d6_roll(3);
    r4 = d6_roll(4);
    d6_roll(9);
    int rollints[4] = {r1, r2, r3, r4};
    int smallest = rollints[0];
    int total = r1 + r2 + r3 + r4;
    for (int i=1; i < sizeof(rollints)/sizeof(rollints[0]); ++i)
        if (rollints[i] < smallest)
            smallest = rollints[i];
    final_total = total - smallest;
    return final_total;
}

int stat_array(int char_stat){
    s1 = stat_roll(1);
    s2 = stat_roll(2);
    s3 = stat_roll(3);
    s4 = stat_roll(4);
    s5 = stat_roll(5);
    s6 = stat_roll(6);
    charstat_1 = s1;
    charstat_2 = s2;
    charstat_3 = s3;
    charstat_4 = s4;
    charstat_5 = s5;
    charstat_6 = s6;
    return char_stat;
}

//Load standard values into stat array
void standard_array(){
    charstat_1 = 15;
    charstat_2 = 14;
    charstat_3 = 13;
    charstat_4 = 12;
    charstat_5 = 10;
    charstat_6 = 8;
}

//Truly random number generator
void truerand(){
    minstd_rand simple_rand;
    simple_rand.seed(42);
    for (int ii = 0; ii<10; ++ii);
}

int main() {
    truerand();
    srand(time(NULL));
    stat_array(0);
    int final_total = stat_roll(0);
    cout << "You rolled a " << r1 << ", a " << r2 << ", a " << r3 << ", and a " << r4
         << ", and your ability score roll is " << final_total << ".\n";
    cout << "Your stat values are " << s1 << ", " << s2 << ", " << s3 << ", " << s4 << ", " << s5 << ", and " << s6
         << ".\n";
    cout << "For verification, " << charstat_1 << ", " << charstat_2 << ", " << charstat_3 << ", " << charstat_4 << ", "
         << charstat_5 << ", and " << charstat_6 << ".\n";

    //Main functions of the program here//
}
