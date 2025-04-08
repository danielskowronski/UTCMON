include <display_common.scad>;


module display_board_screw(){
    //TODO: autolmate offset
    //cylinder(wall_thickness, display_board_screw_dia/2, display_board_screw_dia/2);
    color("red")
    translate([0,0,wall_thickness+rnd/3])
    screw_hole("M3,10",head="flat",counterbore=0,anchor=TOP);
}

module distance_sensor_holes(){
    translate([0,0,wall_thickness+rnd/3]) 
        screw_hole("M3,10",head="flat",counterbore=0,anchor=TOP); // bottom screw
    translate([distance_sensor_window_offset_x,distance_sensor_board_distance_between_bolts/2+distance_sensor_window_offset_y-distance_sensor_window_heigh/2,0]){
       // cube([distance_sensor_window_width,distance_sensor_window_heigh, wall_thickness]); // window for sensor
       // cube([distance_sensor_window_width,distance_sensor_window_heigh, wall_thickness]); // window for sensor
        
         color("pink") prismoid(size1=[distance_sensor_window_width-rnd/2,distance_sensor_window_heigh-rnd/2],size2=[distance_sensor_window_width-rnd+wall_thickness,distance_sensor_window_heigh-rnd+wall_thickness], h=wall_thickness*2, $fn=36,anchor=[-1,-1,0], rounding=rnd);//-rnd is caused by quirk of bosl2; -rnd/2 is required for due to small size
        }
        
    translate([0,distance_sensor_board_distance_between_bolts,wall_thickness+rnd/3]) 
        screw_hole("M3,10",head="flat",counterbore=0,anchor=TOP); // top screw
}


module front(){

    difference(){
        base_plate();
        translate([(total_width-display_window_width)/2,(total_height-display_window_height)/2+display_window_offset_y,0]) {
            //cube([display_window_width, display_window_height, wall_thickness*3]);
          //  cuboid([display_window_width, display_window_height, wall_thickness*3], anchor=[-1,-1,-1], rounding=rnd, edges=[BACK+LEFT,FRONT+LEFT,BACK+RIGHT,FRONT+RIGHT]);

            
           // color("pink") cuboid([display_window_width, display_window_height, wall_thickness*1.1], anchor=[-1,-1,0], rounding=-rnd, $fn=36, edges=[TOP]);
            color("pink") prismoid(size1=[display_window_width-rnd, display_window_height-rnd],size2=[display_window_width-rnd+wall_thickness, display_window_height-rnd+wall_thickness], h=wall_thickness*2, $fn=36,anchor=[-1,-1,0], rounding=rnd);//-rnd is caused by quirk of bosl2
            

            
            }
        
        translate([(total_width-back_board_width)/2+distance_sensor_board_offset_from_back_board_x,(total_height-back_board_height)/2+distance_sensor_board_offset_from_back_board_y,0])
            color("red") distance_sensor_holes();
        
        translate([(total_width-display_board_width)/2+display_board_screw_offset_x,(total_height-display_board_height)/2+display_board_screw_offset_y,0]) display_board_screw();
        translate([(total_width+display_board_width)/2-display_board_screw_offset_x,(total_height-display_board_height)/2+display_board_screw_offset_y,0]) display_board_screw();
        translate([(total_width-display_board_width)/2+display_board_screw_offset_x,(total_height+display_board_height)/2-display_board_screw_offset_y,0]) display_board_screw();
        translate([(total_width+display_board_width)/2-display_board_screw_offset_x,(total_height+display_board_height)/2-display_board_screw_offset_y,0]) display_board_screw();
    }
    
    translate([0,0,-wall_full]){
        // bottom wall
        //cube([total_width,wall_thickness, wall_full]);
        cuboid([total_width,wall_thickness, wall_full], anchor=[-1,-1,-1], rounding=rnd, edges=[FRONT+LEFT,FRONT+RIGHT]);
        
        // left wall
        // cube([wall_thickness,total_height, wall_full]);
        cuboid([wall_thickness,total_height, wall_full], anchor=[-1,-1,-1], rounding=rnd, edges=[BACK+LEFT,FRONT+LEFT]);
    }
    
    // top wall
    translate([0,total_height-wall_thickness,-front_wall_top]){
        difference(){
            //cube([total_width,wall_thickness, front_wall_top]);  
            cuboid([total_width,wall_thickness, front_wall_top], anchor=[-1,-1,-1], rounding=rnd, edges=[BACK+LEFT,BACK+RIGHT]);
            translate([(total_width-display_board_width)/2+lux_sensor_window_offset_from_back_board_x,-wall_thickness,0])
            color("red") cube([lux_sensor_window_width,wall_thickness*3,lux_sensor_window_height]);
        }
    }
    
    
    // right wall
    translate([total_width-wall_thickness,0,-front_wall_right]){
        difference(){
            //cube([wall_thickness,total_height, front_wall_right]);
            color("blue") cuboid([wall_thickness,total_height, front_wall_right], anchor=[-1,-1,-1], rounding=rnd, edges=[BACK+RIGHT,FRONT+RIGHT]);
            translate([0,(total_height-back_board_height)/2+connector_offset_from_back_board_z,0])
            color("green") cube([wall_thickness,connector_width,connector_height]);
        }
    }

}

front();