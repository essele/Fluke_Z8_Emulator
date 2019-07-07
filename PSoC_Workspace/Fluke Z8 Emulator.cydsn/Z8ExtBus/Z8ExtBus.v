
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
// Generated on 06/30/2019 at 18:55
// Component: Z8ExtBus
module Z8ExtBus (
	output  AS,
	output  DM,
	output  DS,
	output  DVALID,
	output [3:0] P0,
	output [7:0] P1,
	output  P1_OE,
	output  RW,
	input   CLK,
	input  [15:0] CONTROL
);

//`#start body` -- edit after this line, do not edit this line

// Some of the outputs need to be registered...
reg rw; assign RW = rw;
reg as; assign AS = as;
reg ds; assign DS = ds;
reg p1_oe; assign P1_OE = p1_oe;
reg dvalid; assign DVALID = dvalid;     // TODO: this could be a state based wire?
reg dm; assign DM = dm;
// TODO: do we need ex as a register?

// We will need to keep data state for writes...
reg [7:0] data;

// Special values come in via the control inputs
wire seta; assign seta = CONTROL[15];                   // NOTE: this should be strobed
wire setd; assign setd = CONTROL[14];                   // NOTE: this should be strobed
wire setdm; assign setdm = CONTROL[13];                 // data memory (low for data)
wire setex; assign setex = CONTROL[12];                 // do we want extended bus timing (only for read for now)

// Map a registered AD[11:0] to P0 and P1
reg [11:0] ad; assign P1 = ad[7:0]; assign P0 = ad[11:8];

// State register...
reg [2:0] state;


// Let's go...
always @(posedge CLK) begin

    case (state)
        3'b000: begin   // RESET
                    p1_oe <= 0;
                    rw <= 1;            // Default - inactive high state
                    ds <= 1;            // Not data strobe (inactive high state)
                    as <= 1;            // Not address strobe (inactive high state)
                    dm <= 1;            // Default is non-data (inactive high state)
                    dvalid <= 0;
                    state <= 3'b001;
                end
        3'b001: begin   // SETUP ADDRESS OR DATA (setting data sets mode to write)
                    if (seta) begin
                        ad <= CONTROL[11:0];        // put address on ports
//                        rw <= setrw;                // read or write
                        as <= 0;                    // address strobe low
                        ds <= 1;                    // no change to DS
                        p1_oe <= 1;                 // enable the output
                        dm <= setdm;                // copy setdm to dm
                        state <= 3'b010;
                    end else if (setd) begin
                        data <= CONTROL[7:0];
                        rw <= 0;                    // it's a write if we set data first
                    end
                end
        3'b010: begin   // BRING BACK AS
                    as <= 1;
                    if (rw == 1'b0) begin
                        state <= 3'b100;            // WRITE CYCLE
                    end else begin
                        state <= 3'b011;            // READ CYCLE
                    end
                end
        3'b011: begin   // TRISTATE and take DS low
                        p1_oe <= 0;
                        ds <= 0;
                        if (setex) begin
                            state <= 3'b101;            // extra delay of one cycle if ex
                        end else begin
                            state <= 3'b110;            // for a read, one delay cycle needed
                        end
                end
        3'b100: begin 
                        // WRITE1:   data out, nothing else
                        ad[7:0] <= data;
                        state <= 3'b101;
                end
                
        3'b101: begin
                        // WRITE2:   ds low
                        ds <= 0;
                        state <= 3'b110;
                end
                
        3'b110: begin   // DELAY CYCLE
                    state <= 3'b111;
                end

        3'b111: begin   // DS back high, read DATA, go to beginning
                    ds <= 1;
                    dvalid <= rw;               // read data if it was a read (rw=high)
                    state <= 3'b000;            // back to reset
                end
        default: begin
                 end
    endcase
end

//`#end` -- edit above this line, do not edit this line
endmodule
//`#start footer` -- edit after this line, do not edit this line
//`#end` -- edit above this line, do not edit this line
