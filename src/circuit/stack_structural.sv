module rs_trigger(output wire q, nq, input wire set, s, r, clk, reset);
    wire ns, nr;
    wire rc, sc;
    and a1(rc, r, clk); and a2(sc, s, clk);
    wire s_or_set, r_or_set;
    or a3(s_or_set, set, sc); andn a4(.out(r_or_set), .a(set), .b(rc));
    wire rc_or_reset, sc_or_set;
    or a5(rc_or_reset, r_or_set, reset); andn a6(.out(sc_or_set), .a(reset), .b(s_or_set));
    nor a7(q, rc_or_reset, nq); nor a8(nq, sc_or_set, q);
endmodule

module d_trigger(output wire q, input wire d, clk, reset);
    wire nq;
    wire sc, rc;
    and a1(sc, d, clk); andn a2(.out(rc), .a(d), .b(clk));
    wire s_or_reset, r_or_reset;
    or a3(r_or_reset, rc, reset); andn a4(.out(s_or_reset), .a(reset), .b(sc));
    nor a5(q, r_or_reset, nq); nor a6(nq, s_or_reset, q);
endmodule

module t_trigger(output wire q, nq, input wire set, t, clk, reset);
    wire j, k;
    and a1(j, t, nq); and a2(k, t, q);
    wire nclk;
    not a3(nclk, clk);
    wire q1, nq1;
    rs_trigger rs1(.q(q1), .nq(nq1), .set(set), .s(j), .r(k), .clk(clk), .reset(reset));
    rs_trigger rs2(.q(q), .nq(nq), .set(set), .s(q1), .r(nq1), .clk(nclk), .reset(reset));
endmodule

module Four_bit_cell(output wire[3:0] val, input wire wr, clk, reset, input wire[3:0] data);
    wire edit;
    and a1(edit, wr, clk);
    d_trigger d0(.q(val[0]), .d(data[0]), .clk(edit), .reset(reset));
    d_trigger d1(.q(val[1]), .d(data[1]), .clk(edit), .reset(reset));
    d_trigger d2(.q(val[2]), .d(data[2]), .clk(edit), .reset(reset));
    d_trigger d3(.q(val[3]), .d(data[3]), .clk(edit), .reset(reset));
endmodule

module head(output wire[2:0] out_val, input wire up_down, t, clk, reset);
    reg s12 = 0; wire s3, r1, r2, r, t2, t3;
    wire nq1, nq2, nq3;
    t_trigger trig1(.q(out_val[0]), .nq(nq1), .set(s12), .t(t), .clk(clk), .reset(r1));
    t_trigger trig2(.q(out_val[1]), .nq(nq2), .set(s12), .t(t2), .clk(clk), .reset(r2));
    t_trigger trig3(.q(out_val[2]), .nq(nq3), .set(s3), .t(t3), .clk(clk), .reset(r));
    wire is5, tmp1;
    and a1(tmp1, out_val[0], out_val[2]);
    and a2(s3, tmp1, out_val[1]);
    andn a3(.out(is5), .a(out_val[1]), .b(tmp1));
    or a4(r, reset, is5);
    or a5(r1, r, s3);
    or a6(r2, r, s3);

    wire bor1, bor2;
    and a7(t2, t, bor1);
    and a8(t3, t, bor2);

    wire tmp2, tmp3, tmp4, tmp5;
    and a9(tmp2, out_val[0], up_down);
    andn a10(.out(tmp3), .a(up_down), .b(nq1));
    or a11(bor1, tmp2, tmp3);

    and a12(tmp4, out_val[1], tmp2);
    and a13(tmp5, nq2, tmp3);
    or a14(bor2, tmp4, tmp5);
endmodule

module bit_and(output wire[3:0] out, input wire[3:0] data, input wire b);
    and a1(out[0], data[0], b);
    and a2(out[1], data[1], b);
    and a3(out[2], data[2], b);
    and a4(out[3], data[3], b);
endmodule

module ind_down(output wire[2:0] out, input wire[2:0] data, input wire b);
    and a1(out[0], data[0], b);
    and a2(out[1], data[1], b);
    and a3(out[2], data[2], b);
endmodule

module half_sub(output wire out, borrow, input wire a, b);
    xor a1(out, a, b);
    wire na; not a2(na, a);
    and a3(borrow, na, b);
endmodule

module full_sub(output wire out, borrow_out, input wire a, b, borrow_in);
    wire out1, borrow1, borrow2;
    half_sub hs1(.out(out1), .borrow(borrow1), .a(a), .b(b));
    half_sub hs2(.out(out), .borrow(borrow2), .a(out1), .b(borrow_in));
    or a1(borrow_out, borrow1, borrow2);
endmodule

module get_val(output wire[2:0] out_val, input wire[2:0] head, index);
    wire tmp1; not a0(tmp1, head[0]);
    wire const_1; or a1(const_1, tmp1, head[0]);
    wire h2, h3; xor a2(h2, const_1, head[2]); and a3(h3, const_1, head[2]);

    wire ind_5_or_7; and a4(ind_5_or_7, index[0], index[2]);
    wire tmp2, ind_6; and a5(tmp2, index[1], index[2]); andn a6(.out(ind_6), .a(index[0]), .b(tmp2));
    wire tmp3, tmp4; andn a7(.out(tmp3), .a(ind_5_or_7), .b(index[0])); andn a8(.out(tmp4), .a(ind_5_or_7), .b(index[2]));
    wire ind0, ind1, ind2; or a9(ind0, tmp3, ind_6);
    andn a10(.out(ind1), .a(ind_6), .b(index[1]));
    andn a11(.out(ind2), .a(ind_6), .b(tmp4));

    wire hs_out, hs_borrow;
    half_sub hs(.out(hs_out), .borrow(hs_borrow), .a(head[0]), .b(ind0));
    wire fs1_out, fs1_borrow;
    full_sub fs1(.out(fs1_out), .borrow_out(fs1_borrow), .a(head[1]), .b(ind1), .borrow_in(hs_borrow));
    wire fs2_out, fs2_borrow;
    full_sub fs2(.out(fs2_out), .borrow_out(fs2_borrow), .a(h2), .b(ind2), .borrow_in(fs1_borrow));

    wire out_5_or_7, out_8, out_6;
    and a12(out_5_or_7, hs_out, fs2_out); xor a13(out_8, h3, fs2_borrow);
    wire tmp5; and a14(tmp5, fs1_out, fs2_out); andn a15(.out(out_6), .a(hs_out), .b(tmp5));

    wire tmp6, tmp7; andn a16(.out(tmp6), .a(out_5_or_7), .b(hs_out)); andn a17(.out(tmp7), .a(out_5_or_7), .b(fs2_out));
    wire tmp8; or a18(tmp8, out_6, tmp6);
    wire tmp9; or a19(out_val[0], tmp8, out_8); andn a20(.out(tmp9), .a(out_6), .b(fs1_out)); andn a21(.out(out_val[2]), .a(out_6), .b(tmp7));
    or a22(out_val[1], tmp9, out_8);
endmodule

module demux(output wire[4:0] out, input wire[2:0] index, input wire val);
    wire w11, w12, w13;
    nor a1(w11, index[2], index[1]);
    andn a2(.out(w12), .a(index[2]), .b(index[1]));
    andn a3(.out(w13), .a(index[1]), .b(index[2]));
    wire w21, w22, w23, w24, w25;
    andn a4(.out(w21), .a(index[0]), .b(w11));
    and a5(w22, w11, index[0]);
    andn a6(.out(w23), .a(index[0]), .b(w12));
    and a7(w24, w12, index[0]);
    andn a8(.out(w25), .a(index[0]), .b(w13));
    and a9(out[0], val, w21);
    and a10(out[1], val, w22);
    and a11(out[2], val, w23);
    and a12(out[3], val, w24);
    and a13(out[4], val, w25);
endmodule

module mux(output wire[3:0] out_val, input wire[2:0] index, input wire[3:0] v0, v1, v2, v3, v4);
    wire w1, w2, w3;
    nor a1(w1, index[1], index[2]); andn a2(.out(w2), .a(index[2]), .b(index[1]));
    andn a3(.out(w3), .a(index[1]), .b(index[2]));
    wire ba1_in, ba2_in, ba3_in, ba4_in, ba5_in;
    andn a4(.out(ba1_in), .a(index[0]), .b(w1));
    and a5(ba2_in, w1, index[0]);
    andn a6(.out(ba3_in), .a(index[0]), .b(w2));
    and a7(ba4_in, index[0], w2);
    andn a8(.out(ba5_in), .a(index[0]), .b(w3));
    wire[3:0] ba1_out, ba2_out, ba3_out, ba4_out, ba5_out;
    bit_and ba1(.out(ba1_out), .data(v0), .b(ba1_in));
    bit_and ba2(.out(ba2_out), .data(v1), .b(ba2_in));
    bit_and ba3(.out(ba3_out), .data(v2), .b(ba3_in));
    bit_and ba4(.out(ba4_out), .data(v3), .b(ba4_in));
    bit_and ba5(.out(ba5_out), .data(v4), .b(ba5_in));

    wire[3:0] tmp1, tmp2, tmp3;
    or_4bit a9(.out(tmp1), .in1(ba1_out), .in2(ba2_out));
    or_4bit a10(.out(tmp2), .in1(ba3_out), .in2(ba4_out));
    or_4bit a11(.out(tmp3), .in1(tmp1), .in2(tmp2));
    or_4bit a12(.out(out_val), .in1(ba5_out), .in2(tmp3));
endmodule

module andn(output wire out, input wire a, b);
    wire na; not a1(na, a);
    and a2(out, na, b);
endmodule

module or_4bit(output wire[3:0] out, input wire[3:0] in1, in2);
    or a1(out[0], in1[0], in2[0]);
    or a2(out[1], in1[1], in2[1]);
    or a3(out[2], in1[2], in2[2]);
    or a4(out[3], in1[3], in2[3]);
endmodule

module gate(output wire[3:0] out, input wire[3:0] in, input wire open_n, open_p);
    cmos cm1(out[0], in[0], open_n, open_p);
    cmos cm2(out[1], in[1], open_n, open_p);
    cmos cm3(out[2], in[2], open_n, open_p);
    cmos cm4(out[3], in[3], open_n, open_p);
endmodule

/*
module stack_structural_easy(
    output wire[3:0] O_DATA, 
    input wire RESET, 
    input wire CLK, 
    input wire[1:0] COMMAND, 
    input wire[2:0] INDEX,
    input wire[3:0] I_DATA
    );
    wire edit, push;
    xor a1(edit, COMMAND[0], COMMAND[1]);
    and a2(push, edit, COMMAND[0]);
    wire[2:0] ind;
    head h(.out_val(ind), .up_down(push), .t(edit), .clk(CLK), .reset(RESET));
    wire[4:0] cell_ind;
    demux dm(.out(cell_ind), .index(ind), .val(push));
    wire[3:0] data0, data1, data2, data3, data4;
    Four_bit_cell c0(.val(data0), .wr(cell_ind[0]), .clk(CLK), .reset(RESET), .data(I_DATA));
    Four_bit_cell c1(.val(data1), .wr(cell_ind[1]), .clk(CLK), .reset(RESET), .data(I_DATA));
    Four_bit_cell c2(.val(data2), .wr(cell_ind[2]), .clk(CLK), .reset(RESET), .data(I_DATA));
    Four_bit_cell c3(.val(data3), .wr(cell_ind[3]), .clk(CLK), .reset(RESET), .data(I_DATA));
    Four_bit_cell c4(.val(data4), .wr(cell_ind[4]), .clk(CLK), .reset(RESET), .data(I_DATA));
    wire[2:0] n;
    get_val gv(.out_val(n), .head(ind), .index(INDEX));
    mux m(.out_val(O_DATA), .index(n), .v0(data0), .v1(data1), .v2(data2), .v3(data3), .v4(data4));
endmodule
*/

module open(output wire out, input wire in1, in2, clk);
    wire tmp1, tmp2;
    andn a0(.out(tmp2), .a(in2), .b(in1));
    and a1(tmp1, in1, in2);
    wire tmp3;
    or a2(tmp3, tmp1, tmp2);
    and a3(out, tmp3, clk);
endmodule

module stack_structural_normal(
    inout wire[3:0] IO_DATA, 
    input wire RESET, 
    input wire CLK, 
    input wire[1:0] COMMAND,
    input wire[2:0] INDEX
    ); 
    
    wire edit, push, open_n, open_p;
    open on(.out(open_n), .in1(COMMAND[1]), .in2(COMMAND[0]), .clk(CLK));
    not op(open_p, open_n);
    xor a1(edit, COMMAND[1], COMMAND[0]);
    and a2(push, edit, COMMAND[0]);
    wire[2:0] ind;
    head h(.out_val(ind), .up_down(push), .t(edit), .clk(CLK), .reset(RESET));
    wire[4:0] cell_ind;
    demux dm(.out(cell_ind), .index(ind), .val(push));
    wire[3:0] data0, data1, data2, data3, data4;
    Four_bit_cell c0(.val(data0), .wr(cell_ind[0]), .clk(CLK), .reset(RESET), .data(IO_DATA));
    Four_bit_cell c1(.val(data1), .wr(cell_ind[1]), .clk(CLK), .reset(RESET), .data(IO_DATA));
    Four_bit_cell c2(.val(data2), .wr(cell_ind[2]), .clk(CLK), .reset(RESET), .data(IO_DATA));
    Four_bit_cell c3(.val(data3), .wr(cell_ind[3]), .clk(CLK), .reset(RESET), .data(IO_DATA));
    Four_bit_cell c4(.val(data4), .wr(cell_ind[4]), .clk(CLK), .reset(RESET), .data(IO_DATA));
    wire[2:0] n;
    wire get_by_ind; and gbi(get_by_ind, COMMAND[1], COMMAND[0]);
    wire[2:0] actual_ind;
    ind_down ind_d(.out(actual_ind), .data(INDEX), .b(get_by_ind));
    get_val gv(.out_val(n), .head(ind), .index(actual_ind));
    wire[3:0] data_out;
    mux m(.out_val(data_out), .index(n), .v0(data0), .v1(data1), .v2(data2), .v3(data3), .v4(data4));
    gate g(.out(IO_DATA), .in(data_out), .open_n(open_n), .open_p(open_p));

endmodule
