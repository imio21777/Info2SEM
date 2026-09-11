module mips_tb;

reg reset, clock;

// Change the TopLevel module's name to yours
TopLevel topLevel(.reset(reset), .clock(clock));

integer k;
initial begin
    // posedge clock

    // Hold reset for one cycle
    reset = 1;
    clock = 0; #1;
    clock = 1; #1;
    clock = 0; #1;
    reset = 0; #1;
    
    $stop; // Comment this line if you don't need per-cycle debugging

    #1;
    for (k = 0; k < 5000; k = k + 1) begin // 5000 clocks
        clock = 1; #5;
        clock = 0; #5;
    end

    // Please finish with `syscall`, finishes here may mean the clocks are not enough
    $finish;
end
    
endmodule
