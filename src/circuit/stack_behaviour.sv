/*
module stack_behaviour_easy(
    output wire[3:0] O_DATA, 
    input wire RESET, 
    input wire CLK, 
    input wire[1:0] COMMAND, 
    input wire[2:0] INDEX,
    input wire[3:0] I_DATA
    ); 
    reg[2:0] stack[4:0];
    reg[2:0] top;
    reg[3:0] O_DATA_reg;
    assign O_DATA = CLK ? O_DATA_reg:I_DATA;
    
    always @(posedge RESET) begin
        top = 0;
        for (int i = 0; i < 5; i++) begin
            stack[i] = 'd0;
        end
    end

    always @(posedge CLK) begin
        if (COMMAND == 'd0) begin
            O_DATA_reg = I_DATA;
        end
        else if (COMMAND == 'd1) begin
            O_DATA_reg = I_DATA;
            stack[top] = I_DATA;
            top <= (top + 1) % 5;
        end
        else if (COMMAND == 'd2) begin
            O_DATA_reg = stack[(top + 4)%5];
            top <= (top + 4) % 5;
        end
        else if (COMMAND == 'd3) begin
            O_DATA_reg = stack[(top + 4 - INDEX%5)%5];
        end
    end
endmodule
*/

module stack_behaviour_normal(
    inout wire[3:0] IO_DATA, 
    input wire RESET, 
    input wire CLK, 
    input wire[1:0] COMMAND,
    input wire[2:0] INDEX
    ); 
    reg[3:0] stack[4:0];
    reg[2:0] top;
    reg[3:0] O_DATA_reg;

    assign IO_DATA = (CLK && (COMMAND == 2 || COMMAND == 3)) ? O_DATA_reg : 4'bz;
    always @(posedge RESET) begin
        top = 0;
        for (int i = 0; i < 5; i++) begin
            stack[i] = 'd0;
        end
    end

    always @(posedge CLK) begin
        if (COMMAND == 'd0) begin
            O_DATA_reg = IO_DATA;
        end
        else if (COMMAND == 'd1) begin
            O_DATA_reg = IO_DATA;
            stack[top] = IO_DATA;
            top <= (top + 1) % 5;
        end
        else if (COMMAND == 'd2) begin
            O_DATA_reg = stack[(top + 4)%5];
            top <= (top + 4) % 5;
        end
        else if (COMMAND == 'd3) begin
            O_DATA_reg = stack[(top + 4 - INDEX%5)%5];
        end
    end
    
endmodule