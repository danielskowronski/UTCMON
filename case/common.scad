include <BOSL2/std.scad>
include <BOSL2/screws.scad>

$fa = 1;
$fs = 0.5;
$fn=36;

back_board_screw_dia=2;
back_board_screw_offset=1.75; //2?
back_board_thickness=1.5;

module base_plate_screw(){
    //TODO: autolmate offset
//    cylinder(wall_thickness, back_board_screw_dia/2, back_board_screw_dia/2);
    color("red")
    translate([0,0,wall_thickness+rnd/3])
    screw_hole("M2,10",head="flat",counterbore=0,anchor=TOP);
}

module base_plate(x_offset=0){
    difference(){
        cuboid([total_width, total_height, wall_thickness], anchor=[-1,-1,-1], rounding=rnd, edges=[TOP, "Z"]); // main front/back
        //cube([total_width, total_height, wall_thickness]); // main front/back

        translate([(total_width-back_board_width)/2+back_board_screw_offset+x_offset,(total_height-back_board_height)/2+back_board_screw_offset,0]) base_plate_screw(); 
        translate([(total_width+back_board_width)/2-back_board_screw_offset+x_offset,(total_height-back_board_height)/2+back_board_screw_offset,0]) base_plate_screw();
        translate([(total_width-back_board_width)/2+back_board_screw_offset+x_offset,(total_height+back_board_height)/2-back_board_screw_offset,0]) base_plate_screw();
        translate([(total_width+back_board_width)/2-back_board_screw_offset+x_offset,(total_height+back_board_height)/2-back_board_screw_offset,0]) base_plate_screw();

    }

}