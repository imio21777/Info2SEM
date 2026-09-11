`timescale 1ns / 1ns
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/08 20:38:44
// Design Name: 
// Module Name: alu_tb
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module alu_tb();
    reg [31:0] in1,in2;
    reg [5:0] op;
    reg [31:0] ans;
    reg cov;
    
    wire [31:0] out;
    wire ov;
    
    alu uut(.A(in1),.B(in2),.C(out),.Op(op),.Over(ov));
   
    wire correct;
    assign correct=(out==ans)&&(ov==cov);
    
    integer i,j,ii,jj;
    
    initial begin
        op=6'b100000;
        for(i=0;i<32;i=i+1)
            for(ii=0;ii<16;ii=ii+1)
                for(j=0;j<32;j=j+1)
                    for(jj=0;jj<16;jj=jj+1)
                        begin
                            in1=ii<<i;
                            in2=jj<<j;
                            ans=in1+in2;
                            cov=(in1[31]==in2[31]&&in1[31]!=ans[31]);
                            #2;
                        end;
        #5
        op=6'b100010;
        for(i=0;i<32;i=i+1)
            for(ii=0;ii<16;ii=ii+1)
                for(j=0;j<32;j=j+1)
                    for(jj=0;jj<16;jj=jj+1)
                        begin
                            in1=ii<<i;
                            in2=jj<<j;
                            ans=in1-in2;
                            cov=(in1[31]!=in2[31]&&in1[31]!=ans[31]);
                            #2;
                        end;
        #5
        cov = 0;
        op=6'b100001;
        for(i=0;i<32;i=i+1)
            for(ii=0;ii<4;ii=ii+1)
                for(j=0;j<32;j=j+1)
                    for(jj=0;jj<4;jj=jj+1)
                        begin
                            in1=ii<<i;
                            in2=jj<<j;
                            ans=in1+in2;
                            #2;
                        end;
        #5
        op=6'b100011;
        for(i=0;i<32;i=i+1)
            for(ii=0;ii<4;ii=ii+1)
                for(j=0;j<32;j=j+1)
                    for(jj=0;jj<4;jj=jj+1)
                        begin
                            in1=ii<<i;
                            in2=jj<<j;
                            ans=in1-in2;
                            #2;
                        end;
        #5
        op=6'b000000;
        for(i=0;i<32;i=i+1)
            for(j=0;j<32;j=j+1)
                for(jj=0;jj<128;jj=jj+1)
                    begin
                        in1=i;
                        in2=jj<<j;
                        ans=(in2)<<in1[4:0];
                        #2;
                    end;
        #5
        op=6'b000010;
        for(i=0;i<32;i=i+1)
            for(j=0;j<32;j=j+1)
                for(jj=0;jj<128;jj=jj+1)
                    begin
                        in1=i;
                        in2=jj<<j;
                        ans=(in2)>>in1[4:0];
                        #2;
                    end;      
        #5
        op=6'b000011;
        for(i=0;i<32;i=i+1)
            for(j=0;j<32;j=j+1)
                for(jj=0;jj<128;jj=jj+1)
                    begin
                        in1=i;
                        in2=jj<<j;
                        ans=$signed(in2)>>>in1[4:0];
                        #2;
                    end;
        #5
        op=6'b100100;
        for(i=0;i<32;i=i+1)
            for(ii=0;ii<8;ii=ii+1)
                for(j=0;j<32;j=j+1)
                    for(jj=0;jj<8;jj=jj+1)
                        begin
                            in1=ii<<i;
                            in2=jj<<j;
                            ans=in1&in2;
                            #2;
                        end;
        #5
        op=6'b100101;
        for(i=0;i<32;i=i+1)
            for(ii=0;ii<8;ii=ii+1)
                for(j=0;j<32;j=j+1)
                    for(jj=0;jj<8;jj=jj+1)
                        begin
                            in1=ii<<i;
                            in2=jj<<j;
                            ans=in1|in2;
                            #2;
                        end;
        #5
        op=6'b100110;
        for(i=0;i<32;i=i+1)
            for(ii=0;ii<8;ii=ii+1)
                for(j=0;j<32;j=j+1)
                    for(jj=0;jj<8;jj=jj+1)
                        begin
                            in1=ii<<i;
                            in2=jj<<j;
                            ans=in1^in2;
                            #2;
                        end;
        #5
        op=6'b100111;
        for(i=0;i<32;i=i+1)
            for(ii=0;ii<8;ii=ii+1)
                for(j=0;j<32;j=j+1)
                    for(jj=0;jj<8;jj=jj+1)
                        begin
                            in1=ii<<i;
                            in2=jj<<j;
                            ans=~(in1|in2);
                            #2;
                        end;
        $finish;
    end;
endmodule
