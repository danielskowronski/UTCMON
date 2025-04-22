include <display_common.scad>;

module display_back(){
    base_plate();
    
    //top wall
    translate([wall_thickness,0,-bottom_wall_top]) {
        difference(){
            cube([total_width-2*wall_thickness,wall_thickness, bottom_wall_top]);
            translate([(total_width-back_board_width)/2+lux_sensor_board_offset_from_back_board_x-wall_thickness,wall_thickness-lux_sensor_board_problematic_deth,0])
                color("red")cube([lux_sensor_board_width, lux_sensor_board_problematic_deth,bottom_wall_top ]);
        }
    }
    
    //right wall
    translate([total_width-wall_thickness,0,-bottom_wall_right]) {
        //cube([wall_thickness,total_height-wall_thickness, bottom_wall_right]);
        cuboid([wall_thickness,total_height-wall_thickness, bottom_wall_right], anchor=[-1,-1,-1], rounding=rnd, edges=[FRONT+RIGHT]);
        }
        
}

display_back();