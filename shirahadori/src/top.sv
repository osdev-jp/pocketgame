module top(
  input  logic sys_clk,
  input  logic rst_n_raw,
  input  logic sw_n_raw,
  output logic [5:0] onboard_led,
  output logic [7:0] dbg
);

localparam MOVE_PERIOD = 2_700_000; // 刀LEDが動く速さ

typedef enum logic [2:0] {
  INIT,     // スイッチが押されるのを待っている
  RNG,      // 乱数を生成している
  STANDBY,  // 刀が動き出すのをランダムな時間待っている
  MOVING,   // 刀が動いている
  RESULT    // 刀を受け止めた・失敗した
} state_t;

state_t state;
state_t state_prev;

logic rst_n, sw_n, sw_n_prev;
always @(posedge sys_clk) begin
  rst_n <= rst_n_raw;
  sw_n_prev <= sw_n;
  sw_n <= sw_n_raw;
end

always @(posedge sys_clk, negedge rst_n) begin
  if (!rst_n) begin
    state <= INIT;
    state_prev <= INIT;
  end
  else begin
    state_prev <= state;
    if (state == INIT && !sw_n) begin
      state <= RNG;
    end
    else if (state == RNG && sw_n) begin
      state <= STANDBY;
    end
    else if (state == STANDBY && cnt == 0) begin
      state <= MOVING;
    end
    else if (state == MOVING && !sw_n && sw_n_prev) begin
      state <= RESULT;
    end
  end
end

// 疑似乱数生成
logic [24:0] lfsr;
always @(posedge sys_clk, negedge rst_n) begin
  if (!rst_n) begin
    lfsr <= 1;
  end
  else begin
    if (!sw_n) begin
      lfsr <= {lfsr[23:0],
        lfsr[25-1] ^ lfsr[21-1]
      };
    end
  end
end

logic [24:0] cnt;

always @(posedge sys_clk, negedge rst_n) begin
  if (!rst_n) begin
    cnt <= '1;
  end
  else if (state == RNG && sw_n) begin
    cnt <= lfsr;
  end
  else if (state == STANDBY && cnt > 0) begin
    cnt <= cnt - 1;
  end
end


logic [5:0] current_led;
always @(posedge sys_clk, negedge rst_n) begin
  if (!rst_n) begin
    current_led <= 6'b000001;
  end
  else if (blink_timcnt == MOVE_PERIOD - 1) begin
    current_led <= current_led << 1;
  end
end

logic [24:0] blink_timcnt;
always @(posedge sys_clk, negedge rst_n) begin
  if (!rst_n) begin
    blink_timcnt <= 0;
  end
  else if (cnt == 0) begin
    if (blink_timcnt < MOVE_PERIOD) begin
      blink_timcnt <= blink_timcnt + 1;
    end
    else begin
      blink_timcnt <= 0;
    end
  end
end

logic is_success;
always @(posedge sys_clk, negedge rst_n) begin
  if (!rst_n) begin
    is_success <= 0;
  end
  else if (state == RESULT && state_prev != RESULT) begin   // Resultになった瞬間
    is_success <= current_led[5];                           // 端っこ来た時にスイッチ押せているか
  end
end

assign onboard_led = ~(state == RESULT ? (is_success ? 6'b101011 : 6'b000011) : current_led);
assign dbg = {state, cnt == 0, cnt[2:0]};

endmodule

/*
= 真剣白刃取りゲーム

Tang Nano 9Kの6個のLEDとスイッチ1つを使ったものです。スイッチを押すとLEDの右端が点灯します。-は消灯、*は点灯として、こんな状態。

LED: -----* 

スイッチを離すと、ランダムな時間が経過した後、LEDの点灯位置が徐々に左にずれてきます。

LED: ----*-
LED: ---*--
…
LED: *-----


左端に来たときにちょうどスイッチを押すことができれば、真剣白刃取り成功。早すぎたり遅すぎたりしたら失敗。というゲーム。
スイッチを押す時間にはランダム性があるので、その時間で疑似乱数を回しまくります。スイッチが離された瞬間の乱数値を、LEDが動き出すまでの時間として使います。
*/

/*
LED の点灯箇所をだんだん動かす方法
1. LEDの点灯位置を表す3ビットのカウンタを作り、インクリメント（デクリメント）する
   LED  (1 << CNT)
2. 6ビットのレジスタを作り、1ビットずつシフトさせる
   LED 1 2 3 4 5 6
   REG 0 0 1 0 0 0  ←これをどんどん右にずらす <- とりあえずこちらでやる
*/