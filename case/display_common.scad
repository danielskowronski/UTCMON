include <BOSL2/std.scad>
include <BOSL2/screws.scad>

// design for LEFT display board, mirror in slicer
// 0,0 = left-bottom (side of distance sensor)

$fa = 1;
$fs = 0.5;
$fn=36;

total_width=150;
total_height=40;
wall_thickness=2;
wall_full=21.5; // between inside of front and back = 5+10+5 (distance/standoffs) + 1.5mm (board thickness)

display_window_width=79;
display_window_height=22;
//display_window_offset_from_bottom=7; // FIXME!
display_window_offset_y=-1; // it's not centered

display_board_width=100.6;
display_board_height=33.5;
display_board_screw_dia=3;
display_board_screw_offset_x=2.825; //??
display_board_screw_offset_y=2.0; //??

back_board_width=140.25;
back_board_height=30;
back_board_screw_dia=2;
back_board_screw_offset=1.75; //2?

distance_sensor_board_offset_from_back_board_x=9; 
distance_sensor_board_offset_from_back_board_y=0;
distance_sensor_board_distance_between_bolts=20;
distance_sensor_window_heigh=4.5;
distance_sensor_window_width=2.5;
distance_sensor_window_offset_x=0.4;
distance_sensor_window_offset_y=-0.1;


lux_sensor_window_offset_from_back_board_x=-10.5;
lux_sensor_window_offset_from_front=4;
lux_sensor_window_width=3;
lux_sensor_window_height=5;

lux_sensor_board_offset_from_back_board_x=3; // true: 4
lux_sensor_board_width=15.5; // true: 14
lux_sensor_board_problematic_deth=1;

connector_height=9;
connector_width=20.75; // <-- was 20.5
connector_offset_from_back_board_z=3.5;

front_wall_right = 14.5;
front_wall_left = wall_full;
front_wall_bottom = wall_full;
front_wall_top = lux_sensor_window_offset_from_front+lux_sensor_window_height;

bottom_wall_right =  wall_full-front_wall_right;
bottom_wall_top =  wall_full-front_wall_top;

rnd = wall_thickness/2;

module base_plate_screw(){
    //TODO: autolmate offset
//    cylinder(wall_thickness, back_board_screw_dia/2, back_board_screw_dia/2);
    color("red")
    translate([0,0,wall_thickness+rnd/3])
    screw_hole("M2,10",head="flat",counterbore=0,anchor=TOP);
}

module base_plate(){
    difference(){
        cuboid([total_width, total_height, wall_thickness], anchor=[-1,-1,-1], rounding=rnd, edges=[TOP, "Z"]); // main front/back
        //cube([total_width, total_height, wall_thickness]); // main front/back

        translate([(total_width-back_board_width)/2+back_board_screw_offset,(total_height-back_board_height)/2+back_board_screw_offset,0]) base_plate_screw(); 
        translate([(total_width+back_board_width)/2-back_board_screw_offset,(total_height-back_board_height)/2+back_board_screw_offset,0]) base_plate_screw();
        translate([(total_width-back_board_width)/2+back_board_screw_offset,(total_height+back_board_height)/2-back_board_screw_offset,0]) base_plate_screw();
        translate([(total_width+back_board_width)/2-back_board_screw_offset,(total_height+back_board_height)/2-back_board_screw_offset,0]) base_plate_screw();

    }

}
