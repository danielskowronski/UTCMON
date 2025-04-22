include <main_common.scad>

module main_front(){
    base_plate(base_plate_screws_x_offset);
    
    
    //top wall
    translate([wall_thickness,0,-front_wall_top]) {
            cube([total_width-2*wall_thickness,wall_thickness, front_wall_top]);
        
    }
    //bottom wall
    translate([wall_thickness,total_height-wall_thickness,-front_wall_top]) {
            cube([total_width-2*wall_thickness,wall_thickness, front_wall_bottom]);
        
    }
    
    //right wall
    translate([total_width-wall_thickness,0,-front_wall_right]) {
        cuboid([wall_thickness,total_height, front_wall_right], anchor=[-1,-1,-1], rounding=rnd, edges=[FRONT+RIGHT, BACK+RIGHT]);
        }
        
    //left wall
    translate([0,0,-front_wall_left]) {
        cuboid([wall_thickness,total_height, front_wall_left], anchor=[-1,-1,-1], rounding=rnd, edges=[FRONT+LEFT, BACK+LEFT]);
        }
        
}

main_front();