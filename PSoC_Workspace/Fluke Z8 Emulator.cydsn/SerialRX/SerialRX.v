
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
// Generated on 07/24/2019 at 12:35
// Component: SerialRX
module SerialRX (
	output [7:0] dout,
	output  irq,
	input   clk,
	input   rx
);

//`#start body` -- edit after this line, do not edit this line

reg         running;    // have we seen a start signal
reg         dv;         // data valid (becomes irq)

//assign      irq = dv;


// If we use a 7 bit counter we can use an 8Mhz clock ... consider this later
//reg [2:0]   clock_count;
wire        count_enable;
wire        count_load;
wire [6:0]  count;
wire        count_zero;

assign count_enable = 1;

assign irq = dv;

        cy_psoc3_count7 #(.cy_period(7'b1001111),.cy_route_ld(1),.cy_route_en(1)) RxBitCounter
        (
            /*  input             */  .clock(clk),
            /*  input             */  .reset(1'b0),
            /*  input             */  .load(count_load),
            /*  input             */  .enable(count_enable),
            /*  output  [06:00]   */  .count(count),
            /*  output            */  .tc(count_zero)
        );

reg [9:0]   data;

assign dout = { data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8] };        // reverse
//assign dout = { running, count[6:0] };


wire [2:0]  bit_pos;
wire [3:0]  bit_index;

assign bit_index = count[6:3];
assign bit_pos = count[2:0];

parameter IDLE      = 3'b000;
parameter RXSTART   = 3'b001;
parameter RXDATA    = 3'b010;
parameter RXSTOP    = 3'b011;
parameter CLEANUP   = 3'b100;

// Don't run the counter when we are waiting for a start signal
assign count_load = !running;


always @(posedge clk) begin

    case (running)
    
        1'b0:   begin
                    dv <= 0;
                    
                    if (rx == 1'b0)
                        running <= 1;
                    else
                        running <= 0;
                end

        1'b1:   begin
                    if (bit_pos == 3'b100) begin    // half way through a bit
                        data[bit_index] <= rx;
                        
                        if (bit_index == 4'b1001) begin
                            if (rx != 1'b0) begin
                                running <= 0;
                            end
                        end
                    end
                    
                    if (count_zero) begin
                        dv <= 1;
                        running <= 0;
                    end
                end
    endcase


end


//`#end` -- edit above this line, do not edit this line
endmodule
//`#start footer` -- edit after this line, do not edit this line
//`#end` -- edit above this line, do not edit this line
