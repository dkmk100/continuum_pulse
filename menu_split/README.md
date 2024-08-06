# the readme file for the main code

this area is the arduino project


ROADMAP:

-split up OLED_menu implementation separately, with an OLED_implementation file
-the latter can use define macro BS to make the former easier
-also, make menu not be virtual but do some kind of templates or something
-split tick and frame draw functions on menu
-change menu code to use stack objects instead of using so many heap allocations
-make this true of the screen stack too, maybe even make a screen stack a struct we can make

-eventually make headers for boards and modes and stuff so I can just include that
-instead of remembering pin settings each time

I think that's everything