module mips_tb;

reg reset, clock;

// The top level module name should be always "TopLevel"
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
    
    // This line is commented when testing
    // $stop;

    #1;
    for (k = 0; k < 2000000000; k = k + 1) begin
        clock = 1; #5;
        clock = 0; #5;
    end

    // Please finish with `syscall`, finishes here may mean the clocks are not enough (really?)
    $finish;
end
    
endmodule
