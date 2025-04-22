include <main_common.scad>

module main_back(){
    base_plate(base_plate_screws_x_offset);
    
    
    //top wall
    translate([wall_thickness,0,-back_wall_top]) {
            cube([total_width-2*wall_thickness,wall_thickness, back_wall_top]);
        
    }
    //bottom wall
    translate([wall_thickness,total_height-wall_thickness,-back_wall_top]) {
            cube([total_width-2*wall_thickness,wall_thickness, back_wall_bottom]);
        
    }
    
    //right wall
    translate([total_width-wall_thickness,0,-back_wall_right]) {
        cuboid([wall_thickness,total_height, back_wall_right], anchor=[-1,-1,-1], rounding=rnd, edges=[FRONT+RIGHT, BACK+RIGHT]);
    }
        
    //left wall
    translate([0,0,-back_wall_left]) {
      difference(){
        cuboid([wall_thickness,total_height, back_wall_left], anchor=[-1,-1,-1], rounding=rnd, edges=[FRONT+LEFT, BACK+LEFT]);
        translate([0,(total_height-back_wall_left_hole_width)/2,0])
        color("red") cube([wall_thickness,back_wall_left_hole_width,back_wall_left_hole_height]);
        }
      }
        
}

main_back();