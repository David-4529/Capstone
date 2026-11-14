% generate_figures.m  (Rev 8)
% All figures black/white/grayscale only - datasets distinguished by marker
% shape and line style, not color, per IEEE black-and-white formatting requirement.
%
% Sensorless Load Estimation Using Motor Slip on a Low-Cost VFD-Driven Induction Motor
% Single script, regenerates every figure in the paper/deck directly from logged data.
%
% Run from the repo root (or edit RESULTS_GLOB below):
%   octave tools/matlab/generate_figures.m
% or in MATLAB:
%   generate_figures
%
% Reads:  results/*/analysis/groups.csv   (written by tools/logger analysis scripts -
%                                           one row per freq/direction/weight, already
%                                           averaged over each run's settled plateau)
% Writes: tools/matlab/figures/*.png
%
% Output -> paper/deck figure number map (by order of appearance in the paper):
%   fig1_block_diagram.png        Fig. 1  (schematic, no data - drawn, not plotted)
%   (not produced here)           Fig. 2  (Drum_Rev_0 CAD render - see
%                                           paper_rev0/build_paper.js's drum_cad.png)
%   fig2_rs485_wiring.png         Fig. 4  (schematic, no data - drawn, not plotted;
%                                           filename says fig2 because it was authored
%                                           before the drum render's place in the
%                                           document order was accounted for - the
%                                           figure NUMBER used in the paper is 4)
%   (not produced here)           Fig. 5  (raw USART2 screenshot - see
%                                           paper_rev0/assets/raw_csv_screenshot.png)
%   fig4_baseline_combined.png    Fig. 6  (no-load baseline, up direction only - Rev 10
%                                           dropped down-direction data entirely; see
%                                           note below)
%   fig5_calibration_fit.png      Fig. 7
%   fig6_blind_validation.png     Fig. 8
%   fig7_loocv_model_comparison.png Fig. 10
%   fig8_torque_validation.png    Fig. 9  (independent torque-sensor validation; Rev 10
%                                           converted its torque axis from mN*m to N*m)
%
% Fig. 1/2 are drawn schematics (wiring per docs/wiring.md + Table III). Fig. 6/7 are
% computed directly from the real groups.csv files checked into the repo (genuine
% recomputation, not hardcoded). Fig. 8/10 reproduce the already-reported blind-
% validation and LOOCV results (Tables VIII/XI of the paper) - those specific held-out
% sessions were analyzed ad hoc and are not re-derivable from a groups.csv in this
% repo, so their numbers are transcribed here from the paper with a comment at each
% one, not recomputed.
%
% Rev 10: down-direction (lowering) slip/current data removed from Fig. 6 entirely -
% the motor has no dynamic braking, so down-direction readings were never physically
% meaningful and were never used in the paper's model (David's call, Rev 9 review).

clear; clc; close all;

% Paper figures get embedded small (IEEE two-column), so default text/axes
% sizes are bumped well above Octave's defaults to stay legible there -
% flagged as too small to read in Rev 3 review comments.
set(0, 'DefaultAxesFontSize', 15);
set(0, 'DefaultTextFontSize', 15);
set(0, 'DefaultLineLineWidth', 2.2);
set(0, 'DefaultAxesLineWidth', 1.1);

THIS_DIR = fileparts(mfilename('fullpath'));
REPO_ROOT = fileparts(fileparts(THIS_DIR)); % tools/matlab -> tools -> repo root
RESULTS_GLOB = fullfile(REPO_ROOT, 'results', '*', 'analysis', 'groups.csv');
OUT_DIR = fullfile(THIS_DIR, 'figures');
if ~exist(OUT_DIR, 'dir'); mkdir(OUT_DIR); end

TARGET_FREQS = [1.39, 2.44, 4.85, 6.36];
POLE_COUNT = 4;

files = glob(RESULTS_GLOB);
if isempty(files)
  error('No groups.csv files found under %s - run from the repo root.', RESULTS_GLOB);
end
printf('Found %d groups.csv file(s):\n', numel(files));
for i = 1:numel(files); printf('  %s\n', files{i}); end

% ---- Load every row from every session into one flat table ----
freq_hz = []; direction = {}; total_kg = []; current_mean = []; slip_mean = [];

for fi = 1:numel(files)
  fid = fopen(files{fi}, 'r');
  header = fgetl(fid); %#ok<NASGU> - freq_hz,direction,total_kg,n,slip_mean,...
  while true
    csv_line = fgetl(fid);
    if ~ischar(csv_line); break; end
    % collapsedelimiters must be false - several rows have blank slip_* fields
    % (e.g. results/2026-10-24_resistorx2/analysis/groups.csv), and the default
    % (true) silently merges consecutive commas, shifting every later column.
    parts = strsplit(csv_line, ',', 'collapsedelimiters', false);
    if numel(parts) < 11; continue; end
    f = str2double(parts{1});
    d = parts{2};
    w = str2double(parts{3});
    sm = str2double(parts{5});
    cm = str2double(parts{9});
    if isnan(f) || isnan(w); continue; end
    freq_hz(end+1) = f; %#ok<AGROW>
    direction{end+1} = d; %#ok<AGROW>
    total_kg(end+1) = w; %#ok<AGROW>
    slip_mean(end+1) = sm; %#ok<AGROW>
    current_mean(end+1) = cm; %#ok<AGROW>
  end
  fclose(fid);
end
printf('Loaded %d total freq/direction/weight rows.\n', numel(freq_hz));

nearest_target = @(f) TARGET_FREQS(find(abs(TARGET_FREQS - f) == min(abs(TARGET_FREQS - f)), 1));
bucket = arrayfun(nearest_target, freq_hz);

% ============================================================================
% Figure 1: System signal-path block diagram
% Drawn (rectangle()/text()/line()), not a data plot - no CSV input.
% ============================================================================
fig = figure('Visible', 'off', 'Position', [0 0 1000 380]);
axis([0 10 0 3.6]); axis off; hold on;
box_h = 1.2;
boxes = {
  {0.3, 'PC Logger\n(operator HMI)', [0.88 0.88 0.88]}, ...
  {2.5, 'STM32\nNucleo-F401RE', [0.65 0.65 0.65]}, ...
  {4.7, 'RS-485\nMAX485', [0.78 0.78 0.78]}, ...
  {6.6, 'VFD\n(DURApulse\nGS23-21P0)', [0.65 0.65 0.65]}, ...
  {8.6, 'Motor + Load', [0.88 0.88 0.88]}
};
box_w = 1.7;
centers = zeros(1, numel(boxes));
for i = 1:numel(boxes)
  x0 = boxes{i}{1};
  rectangle('Position', [x0, 1.2, box_w, box_h], 'FaceColor', boxes{i}{3}, 'LineWidth', 1.5);
  text(x0 + box_w/2, 1.2 + box_h/2, boxes{i}{2}, 'HorizontalAlignment', 'center', ...
       'VerticalAlignment', 'middle', 'FontSize', 10, 'FontWeight', 'bold');
  centers(i) = x0 + box_w/2;
end
for i = 1:numel(boxes)-1
  x1 = boxes{i}{1} + box_w; x2 = boxes{i+1}{1};
  y = 1.2 + box_h/2;
  line([x1 x2], [y y], 'Color', 'k', 'LineWidth', 1.5);
  line([x2-0.15 x2], [y+0.08 y], 'Color', 'k', 'LineWidth', 1.5);
  line([x2-0.15 x2], [y-0.08 y], 'Color', 'k', 'LineWidth', 1.5);
end
text(centers(1), 2.75, 'USB', 'HorizontalAlignment', 'center', 'FontSize', 9);
text(centers(2)+1.1, 2.75, 'UART +\nDE/RE', 'HorizontalAlignment', 'center', 'FontSize', 9);
text(centers(3)+0.95, 2.75, 'RS-485\n(A/B)', 'HorizontalAlignment', 'center', 'FontSize', 9);
text(centers(4)+1.0, 2.75, 'U/V/W\n3-phase', 'HorizontalAlignment', 'center', 'FontSize', 9);
rectangle('Position', [7.3, 0.05, 1.7, 0.75], 'FaceColor', [0.95 0.95 0.95], 'LineStyle', '--', 'LineWidth', 1);
text(8.15, 0.42, 'Hanging scale\n(manual, cross-check)', 'HorizontalAlignment', 'center', 'FontSize', 8);
line([8.15 8.15], [0.8 1.2], 'Color', 'k', 'LineStyle', '--', 'LineWidth', 1);
saveas(fig, fullfile(OUT_DIR, 'fig1_block_diagram.png'));
close(fig);

% ============================================================================
% Figure 2: RS-485/MAX485 wiring diagram (VFD RJ45 -> MAX485 -> STM32 GPIO)
% Drawn (rectangle()/text()/line()), not a data plot - no CSV input.
% Pin mapping per Table III (paper) / docs/wiring.md section 2.3.
% ============================================================================
fig = figure('Visible', 'off', 'Position', [0 0 1000 500]);
axis([0 10 0 5]); axis off; hold on;

% Three boxes: VFD RJ45, MAX485 module, STM32 Nucleo
rectangle('Position', [0.3, 1.6, 2.3, 1.8], 'FaceColor', [0.65 0.65 0.65], 'LineWidth', 1.5);
text(1.45, 3.0, 'VFD RJ45\n(RS-485 port)', 'HorizontalAlignment', 'center', 'FontWeight', 'bold', 'FontSize', 13);
rectangle('Position', [3.85, 1.0, 2.3, 3.0], 'FaceColor', [0.78 0.78 0.78], 'LineWidth', 1.5);
text(5.0, 3.65, 'MAX485', 'HorizontalAlignment', 'center', 'FontWeight', 'bold', 'FontSize', 13);
rectangle('Position', [7.4, 1.6, 2.3, 1.8], 'FaceColor', [0.65 0.65 0.65], 'LineWidth', 1.5);
text(8.55, 3.0, 'STM32\nNucleo-F401RE', 'HorizontalAlignment', 'center', 'FontWeight', 'bold', 'FontSize', 13);

% VFD <-> MAX485 pin pairs (Table III). Signal names only, no wire colors -
% colors are documented separately in docs/wiring.md for anyone re-wiring it;
% this figure is the signal map.
vfd_pins = {'A', 'B', 'GND'};
for k = 1:3
  y = 2.9 - (k-1)*0.55;
  line([2.6 3.85], [y y], 'Color', 'k', 'LineWidth', 1.5);
  text(3.2, y+0.17, vfd_pins{k}, 'HorizontalAlignment', 'center', 'FontSize', 11, 'FontWeight', 'bold');
end

% MAX485 <-> STM32 pin pairs
stm_pins = {'VCC -> 3.3V', 'DI -> D8', 'DE -> D7', 'RE -> D7', 'RO -> D2'};
for k = 1:5
  y = 3.7 - (k-1)*0.55;
  line([6.15 7.4], [y y], 'Color', 'k', 'LineWidth', 1.5);
  text(6.775, y+0.17, stm_pins{k}, 'HorizontalAlignment', 'center', 'FontSize', 10.5);
end

text(5.0, 0.55, 'DE and RE tied together to one GPIO (D7) - asserted only while transmitting', ...
     'HorizontalAlignment', 'center', 'FontSize', 10.5, 'FontAngle', 'italic');
saveas(fig, fullfile(OUT_DIR, 'fig2_rs485_wiring.png'));
close(fig);

% ============================================================================
% Figure 6: No-load baseline - slip and current vs. frequency, two panels,
% up direction only (Rev 10: down-direction dropped entirely - no dynamic
% braking on this motor, so down-direction readings were never physically
% meaningful and were never used in the paper's model).
% (Fig. 2 is the Drum_Rev_0 CAD render, produced separately - not by this script)
% ============================================================================
fig = figure('Visible', 'off', 'Position', [0 0 1300 520]);
is_noload = (total_kg == 0);

% Plot against the fixed 4-point frequency grid, NaN-filled where no no-load
% run exists at that freq, so a missing point breaks the line (no false
% interpolation) instead of silently connecting across the gap.
grid_slip_up = nan(1, numel(TARGET_FREQS));
grid_cur_up = grid_slip_up;
for k = 1:numel(TARGET_FREQS)
  tf = TARGET_FREQS(k);
  mu = is_noload & strcmp(direction, 'up') & (bucket == tf);
  if any(mu); grid_slip_up(k) = slip_mean(find(mu, 1)); grid_cur_up(k) = current_mean(find(mu, 1)); end
end

subplot(1, 2, 1); hold on;
plot(TARGET_FREQS, grid_slip_up, '-o', 'LineWidth', 2, 'MarkerSize', 8, 'Color', 'k', 'MarkerFaceColor', 'k');
xlabel('Commanded Frequency (Hz)'); ylabel('Slip (fractional)');
title('(a) Slip vs. Frequency'); grid on;

subplot(1, 2, 2); hold on;
plot(TARGET_FREQS, grid_cur_up, '-o', 'LineWidth', 2, 'MarkerSize', 8, 'Color', 'k', 'MarkerFaceColor', 'k');
xlabel('Commanded Frequency (Hz)'); ylabel('Output Current (A)');
title('(b) Output Current vs. Frequency'); grid on;

saveas(fig, fullfile(OUT_DIR, 'fig4_baseline_combined.png'));
close(fig);

% ============================================================================
% Figure 7 (paper Fig. 7): Calibrated model - current vs load, up-direction, one fit per frequency
% ============================================================================
fig = figure('Visible', 'off', 'Position', [0 0 1000 680]);
hold on;
markers = {'o', 's', '^', 'd'};
linestyles = {'-', '--', '-.', ':'};
facecolors = {'k', 'k', 'k', 'k'};
is_up = strcmp(direction, 'up');
coeffs = struct();
for k = 1:numel(TARGET_FREQS)
  tf = TARGET_FREQS(k);
  mask = is_up & (bucket == tf);
  x = current_mean(mask); y = total_kg(mask);
  valid = ~isnan(x) & ~isnan(y);
  x = x(valid); y = y(valid);
  if numel(x) < 2; continue; end
  p = polyfit(x, y, 1); % p(1) = slope, p(2) = intercept
  coeffs.(sprintf('f%d', k)) = p;
  xs = linspace(min(x) - 0.02, max(x) + 0.02, 20);
  ys = polyval(p, xs);
  plot(x, y, markers{k}, 'MarkerSize', 10, 'MarkerFaceColor', facecolors{k}, 'MarkerEdgeColor', 'k', ...
       'DisplayName', sprintf('%.2f Hz data', tf));
  plot(xs, ys, linestyles{k}, 'Color', 'k', 'LineWidth', 1.5, 'HandleVisibility', 'off');
  printf('Fit %.2f Hz: load_kg = %.3f * current_A + %.3f  (n=%d)\n', tf, p(1), p(2), numel(x));
end
xlabel('Output Current (A)'); ylabel('Load (kg)');
title('Calibrated Model: load_{kg} = slope \times current_A + intercept (up direction)');
legend('Location', 'northwest'); grid on;
saveas(fig, fullfile(OUT_DIR, 'fig5_calibration_fit.png'));
close(fig);

% ============================================================================
% Figure 8 (paper Fig. 8): Blind validation - predicted vs. actual
% Transcribed from Table VIII of the paper (Resistor 1/2, T-housing - these
% sessions' raw data was analyzed ad hoc and is not checked into results/ as a
% groups.csv).
% ============================================================================
actual =    [0.64, 0.64, 0.82, 0.82, 0.54, 0.54];
predicted = [0.27, 0.18, 0.87, 0.00, 0.54, 0.00];
% Note: deliberately no '@' in these labels - Octave's gnuplot backend treats
% it as a text-escape character and mangles the following characters.
labels =    {'Resistor 1, 1.44 Hz','Resistor 1, 6.21 Hz','Resistor 2, 2.90 Hz', ...
             'Resistor 2, 4.66 Hz','T-housing, 5.63 Hz','T-housing, 6.81 Hz'};

fig = figure('Visible', 'off', 'Position', [0 0 950 620]);
hold on;
plot([0 1], [0 1], 'k--', 'LineWidth', 1, 'DisplayName', 'Perfect prediction (y = x)');
plot(actual, predicted, 'o', 'MarkerSize', 10, 'MarkerFaceColor', 'k', 'MarkerEdgeColor', 'k', 'DisplayName', 'Blind validation point');
for i = 1:numel(actual)
  text(actual(i) + 0.045, predicted(i), labels{i}, 'FontSize', 11, 'VerticalAlignment', 'middle');
end
xlabel('Actual Load (kg)'); ylabel('Predicted Load (kg)');
title('Blind Validation: Predicted vs. Actual Load');
xlim([0 1.3]); ylim([0 1]); legend('Location', 'northwest'); grid on;
saveas(fig, fullfile(OUT_DIR, 'fig6_blind_validation.png'));
close(fig);

% ============================================================================
% Figure 10 (paper Fig. 10): LOOCV error by model form
% Transcribed from Table XI of the paper (leave-one-out cross-validation across
% all 17 real calibration points - the exact procedure tools/model/fit_model.py
% uses; reproducing it here would require porting that fitting/CV logic to Octave,
% not just replotting its output, so the reported numbers are used directly).
% ============================================================================
model_forms = {'current + freq\n(linear)', '+ interaction\nterm', '+ quadratic\nfreq', 'full\n(all terms)'};
mape = [52.0, 56.8, 56.5, 61.7];

fig = figure('Visible', 'off', 'Position', [0 0 900 620]);
bar(mape, 'FaceColor', [0.75 0.75 0.75], 'EdgeColor', 'k', 'LineWidth', 1.2);
set(gca, 'XTickLabel', model_forms, 'Position', [0.10 0.24 0.86 0.62]);
ylabel('MAPE (%)');
title('LOOCV Error by Model Form');
ylim([0 70]); grid on;
for i = 1:numel(mape)
  text(i, mape(i) + 1.5, sprintf('%.1f%%', mape(i)), 'HorizontalAlignment', 'center');
end
saveas(fig, fullfile(OUT_DIR, 'fig7_loocv_model_comparison.png'));
close(fig);

% ============================================================================
% Figure 9 (paper Fig. 9): Independent torque-sensor validation (SERVO Machine Test System)
% Transcribed from raw ActiveServo (.acts) capture files recorded directly off
% the Lucas Nulle SERVO Machine Test System's own torque/speed sensor in
% Nov 2026 - not in results/*/analysis/groups.csv, so not re-derivable from
% this repo's normal pipeline (same situation as Fig. 6/7). 12 captures total:
% 3 known objects x the same 4 commanded frequencies used throughout
% calibration, ~20-30 marker points per capture. Current is not among the
% channels this sensor captures (speed/slip and torque only).
%
% Slip shown here is the sensor's raw speed channel converted to slip using
% this project's own known Ns = 120*f/P for whatever frequency was actually
% commanded. Torque is read directly off the sensor.
% ============================================================================
tv_freqs = [1.39, 2.44, 4.85, 6.36];
% object, torque (N*m; converted from the raw mN*m sensor readings) by freq,
% slip (%) by freq
tv_objects = {
  {'Plastic housing (0.32 kg)', [0.02892, 0.02884, 0.02802, 0.02891], [40.29, 19.99, 6.41, 5.88]}, ...
  {'Pliers (0.58 kg)',          [0.00979, 0.02875, 0.02798, 0.02829], [35.37, 23.50, 6.49, 7.16]}, ...
  {'Metal housing (1.46 kg)',   [0.02822, 0.02907, 0.02851, 0.02775], [57.37, 30.91, 9.03, 9.75]}  ...
};
tv_markers = {'o', 's', '^'};
tv_linestyles = {'-', '--', '-.'};
tv_facecolors = {'k', 'k', 'k'};

fig = figure('Visible', 'off', 'Position', [0 0 1300 520]);

subplot(1, 2, 1); hold on;
for k = 1:numel(tv_objects)
  plot(tv_freqs, tv_objects{k}{3}, [tv_linestyles{k} tv_markers{k}], 'MarkerSize', 9, 'LineWidth', 1.8, ...
       'Color', 'k', 'MarkerFaceColor', tv_facecolors{k}, 'MarkerEdgeColor', 'k', 'DisplayName', tv_objects{k}{1});
end
xlabel('Commanded Frequency (Hz)'); ylabel('Slip (%)');
title('(a) Slip vs. Frequency'); legend('Location', 'northeast'); grid on;

subplot(1, 2, 2); hold on;
for k = 1:numel(tv_objects)
  plot(tv_freqs, tv_objects{k}{2}, [tv_linestyles{k} tv_markers{k}], 'MarkerSize', 9, 'LineWidth', 1.8, ...
       'Color', 'k', 'MarkerFaceColor', tv_facecolors{k}, 'MarkerEdgeColor', 'k', 'DisplayName', tv_objects{k}{1});
end
xlabel('Commanded Frequency (Hz)'); ylabel('Torque (N\cdotm)');
title('(b) Torque vs. Frequency'); legend('Location', 'southeast'); grid on;

saveas(fig, fullfile(OUT_DIR, 'fig8_torque_validation.png'));
close(fig);

printf('\nAll figures written to %s\n', OUT_DIR);
