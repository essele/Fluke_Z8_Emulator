
//`#start header` -- edit after this line, do not edit this line
// ========================================
//
// Copyright YOUR COMPANY, THE YEAR
// All Rights Reserved
// UNPUBLISHED, LICENSED SOFTWARE.
//
// CONFIDENTIAL AND PROPRIETARY INFORMATION
// WHICH IS THE PROPERTY OF your company.
//
// ========================================
`include "cypress.v"
//`#end` -- edit above this line, do not edit this line
// Generated on 07/25/2019 at 19:08
// Component: TBlock
module TBlock (
	output  d,
	input   clk,
	input   pulse,
	input   set
);

//`#start body` -- edit after this line, do not edit this line

reg data;
reg oldpulse;

assign d = data;

always @(posedge clk) begin

    // if set we always set data, ignore the pulse if we have one


    // if pulse
    //      if oldpulse -- then we are still in the pulse, so do nothing
    //      if not oldpulse -- it's new, so invert data
    //          oldpulse = 1
    // else
    //   oldpulse = 0

    case ({ set, pulse, oldpulse })
    
            
        3'b010 : begin      // pulse leading edge
                    data <= !data;
                    oldpulse <= 1;
                end

        3'b011 : begin      // in an existing pulse
                    oldpulse <= 1;
                end
                
        3'b001 : begin
                    oldpulse <= 0;
                 end
                
        3'b000 : begin      // no incoming pulse
                    oldpulse <= 0;
                 end
    
        default: begin      // anything with set
                    data <= 1;
                end

    
    endcase

end



//        Your code goes here

//`#end` -- edit above this line, do not edit this line
endmodule
//`#start footer` -- edit after this line, do not edit this line
//`#end` -- edit above this line, do not edit this line
