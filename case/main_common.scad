include <common.scad>

total_width=85;
total_height=55;
wall_thickness=2;
rnd = wall_thickness/2;

back_board_width=70;
back_board_height=50;


base_plate_screws_x_offset=-5;

back_wall_left_hole_height=4;
back_wall_left_hole_width=36;

wall_full=5+back_board_thickness+20; // between inside of front and back = 5+20 (distance/standoffs) + 1.5mm (board thickness)

back_wall_top=wall_full;
back_wall_bottom=wall_full;
back_wall_right=wall_full; // THIS ONLY WORKS WITH 180-DEG ELBOW USB-C CABLE, otherwise need to implement hole there
back_wall_left=5+back_board_thickness+8.75+back_wall_left_hole_height;

front_wall_top=wall_full-back_wall_top;
front_wall_bottom=wall_full-back_wall_bottom;
front_wall_right=wall_full-back_wall_right;// THIS ONLY WORKS WITH 180-DEG ELBOW USB-C CABLE, otherwise need to implement hole there
front_wall_left=wall_full-back_wall_left; // 4mm is thickest cable dia -> make it 0.5mm for flat cable only if not using elbow usb cable

