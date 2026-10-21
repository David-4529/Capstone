% NLR WIND Toolkit download for Site A (Perquimans Co., NC), based on the Lecture 1 script.
% Downloads 2007-2014 at 80/100/120 m plus 100 m direction, temperature and pressure.
% Fill in your API key and email (sign up at the NLR Developer site).

api_key = 'YOUR_API_KEY';
email   = 'you@university.edu';
url     = 'https://developer.nlr.gov/api/wind-toolkit/v2/wind/wtk-download.csv';

wkt   = 'POINT(-76.447 36.133)';   % POINT(longitude latitude) -- WTK grid point gid 2266420
attrs = 'windspeed_80m,windspeed_100m,windspeed_120m,winddirection_100m,temperature_100m,pressure_100m';
years = 2007:2014;                 % at least one full year is required; all 8 were used in the report

options = weboptions('Timeout', 120);
for y = years
    query = ['?api_key=' api_key ...
             '&wkt=' wkt ...
             '&names=' num2str(y) ...
             '&attributes=' attrs ...
             '&interval=60' ...    % hourly
             '&utc=true' ...       % timestamps in UTC (convert to local time for Section 3.5)
             '&email=' email];
    filename = sprintf('siteA_wtk_%d.csv', y);
    websave(filename, [url query], options);
    fprintf('Downloaded %s\n', filename);
    pause(2);                      % stay under the API rate limit
end
disp('Download complete.')

% Quick check against the report (Section 3): the 8-year means should be about
% 6.52 m/s at 80 m and 6.91 m/s at 100 m. The CSVs have 1 metadata row before the column headers.
v80 = []; v100 = [];
for y = years
    T = readtable(sprintf('siteA_wtk_%d.csv', y), 'NumHeaderLines', 1);
    v80  = [v80;  T.windSpeedAt80m_m_s_];   %#ok<AGROW>  (column names may differ slightly; check T.Properties.VariableNames)
    v100 = [v100; T.windSpeedAt100m_m_s_];  %#ok<AGROW>
end
alpha = log(mean(v100) / mean(v80)) / log(100 / 80);
v95   = v100 * (95 / 100)^alpha;
pd    = fitdist(v95(v95 > 0), 'Weibull');   % A = scale a, B = shape k
fprintf('mean80 = %.2f, mean100 = %.2f, alpha = %.3f, k = %.2f, a = %.2f m/s\n', mean(v80), mean(v100), alpha, pd.B, pd.A);
