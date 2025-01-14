function example_multiband_drc()

prm.name = "default";
prm.sample_rate = 48000;
prm.enable_emp_deemp = 1;
prm.num_bands = 3;
prm.enable_bands = [1 2 3 0];
prm.band_lower_freq = [0 200 2000 10000];
export_multiband_drc(prm)

prm.name = "passthrough";
prm.sample_rate = 48000;
prm.enable_emp_deemp = 0;
prm.num_bands = 2;
prm.enable_bands = [0 0 0 0];
export_multiband_drc(prm)

end

function export_multiband_drc(prm)

% Set the parameters here
tplg1_fn = sprintf("../../topology/topology1/m4/multiband_drc_coef_%s.m4", prm.name); % Control Bytes File
tplg2_fn = sprintf("../../topology/topology2/include/components/multiband_drc/%s.conf", prm.name); % Control Bytes File
% Use those files with sof-ctl to update the component's configuration
blob3_fn = sprintf("../../ctl/ipc3/multiband_drc/%s.blob", prm.name); % Blob binary file
alsa3_fn = sprintf("../../ctl/ipc3/multiband_drc/%s.txt", prm.name); % ALSA CSV format file
blob4_fn = sprintf("../../ctl/ipc4/multiband_drc/%s.blob", prm.name); % Blob binary file
alsa4_fn = sprintf("../../ctl/ipc4/multiband_drc/%s.txt", prm.name); % ALSA CSV format file

endian = "little";

sample_rate = prm.sample_rate;
nyquist = sample_rate / 2;

% Number of bands, valid values: 1,2,3,4
num_bands = prm.num_bands;

% 1 to enable Emphasis/Deemphasis filter, 0 to disable it
enable_emp_deemp = prm.enable_emp_deemp;

% The parameters of Emphasis IIR filter
% stage_gain: The gain of each emphasis filter stage
iir_params.stage_gain = 4.4;
% stage_ratio: The frequency ratio for each emphasis filter stage to the
%              previous stage
iir_params.stage_ratio = 2;
% anchor: The frequency of the first emphasis filter, in normalized frequency
%         (in [0, 1], relative to half of the sample rate)
iir_params.anchor = 15000 / nyquist;

% The parameters of the DRC compressor
%   enabled: 1 to enable the compressor, 0 to disable it
%   threshold: The value above which the compression starts, in dB
%   knee: The value above which the knee region starts, in dB
%   ratio: The input/output dB ratio after the knee region
%   attack: The time to reduce the gain by 10dB, in seconds
%   release: The time to increase the gain by 10dB, in seconds
%   pre_delay: The lookahead time for the compressor, in seconds
%   release_zone[4]: The adaptive release curve parameters
%   release_spacing: The value of spacing per frame while releasing, in dB
%   post_gain: The static boost value in output, in dB
%   band_lower_freq: The lower frequency of the band, in normalized frequency
%                    (in [0, 1], relative to half of the sample rate)

% Band 1 DRC parameter
drc_params(1).enabled = prm.enable_bands(1);
drc_params(1).threshold = -24;
drc_params(1).knee = 30;
drc_params(1).ratio = 12;
drc_params(1).attack = 0.003;
drc_params(1).release = 0.2;
drc_params(1).pre_delay = 0.006;
drc_params(1).release_zone = [0.09 0.16 0.42 0.98];
drc_params(1).release_spacing = 5;
drc_params(1).post_gain = 0;
drc_params(1).band_lower_freq = prm.band_lower_freq(1) / nyquist;

% Band 2 DRC parameter (only valid if num_bands > 1)
drc_params(2).enabled = prm.enable_bands(2);
drc_params(2).threshold = -24;
drc_params(2).knee = 30;
drc_params(2).ratio = 12;
drc_params(2).attack = 0.003;
drc_params(2).release = 0.2;
drc_params(2).pre_delay = 0.006;
drc_params(2).release_zone = [0.09 0.16 0.42 0.98];
drc_params(2).release_spacing = 5;
drc_params(2).post_gain = 0;
drc_params(2).band_lower_freq = prm.band_lower_freq(2) / nyquist;

% Band 3 DRC parameter (only valid if num_bands > 2)
drc_params(3).enabled = prm.enable_bands(3);
drc_params(3).threshold = -24;
drc_params(3).knee = 30;
drc_params(3).ratio = 12;
drc_params(3).attack = 0.003;
drc_params(3).release = 0.2;
drc_params(3).pre_delay = 0.006;
drc_params(3).release_zone = [0.09 0.16 0.42 0.98];
drc_params(3).release_spacing = 5;
drc_params(3).post_gain = 0;
drc_params(3).band_lower_freq = prm.band_lower_freq(3) / nyquist;

% Band 4 DRC parameter (only valid if num_bands > 3)
drc_params(4).enabled = prm.enable_bands(4);
drc_params(4).threshold = -24;
drc_params(4).knee = 30;
drc_params(4).ratio = 12;
drc_params(4).attack = 0.003;
drc_params(4).release = 0.2;
drc_params(4).pre_delay = 0.006;
drc_params(4).release_zone = [0.09 0.16 0.42 0.98];
drc_params(4).release_spacing = 5;
drc_params(4).post_gain = 0;
drc_params(4).band_lower_freq = prm.band_lower_freq(4) / nyquist;

% Generate Emphasis/Deemphasis IIR filter quantized coefs struct from parameters
[emp_coefs, deemp_coefs] = iir_gen_quant_coefs(iir_params);

% Generate Crossover quantized coefs struct from parameters
crossover_coefs = crossover_gen_quant_coefs(num_bands, sample_rate, ...
					    drc_params(2).band_lower_freq, ...
					    drc_params(3).band_lower_freq, ...
					    drc_params(4).band_lower_freq);

% Generate DRC quantized coefs struct from parameters
drc_coefs = drc_gen_quant_coefs(num_bands, sample_rate, drc_params);

% Generate output files
addpath ../common

% Convert quantized coefs structs to binary blob
blob8 = multiband_drc_build_blob(num_bands, enable_emp_deemp, emp_coefs, ...
				 deemp_coefs, crossover_coefs, drc_coefs, ...
				 endian, 3);
blob8_ipc4 = multiband_drc_build_blob(num_bands, enable_emp_deemp, emp_coefs, ...
				      deemp_coefs, crossover_coefs, drc_coefs, ...
				      endian, 4);

tplg_write(tplg1_fn, blob8, "MULTIBAND_DRC");
tplg2_write(tplg2_fn, blob8_ipc4, "multiband_drc_config", 'Exported Control Bytes');
blob_write(blob3_fn, blob8);
alsactl_write(alsa3_fn, blob8);
blob_write(blob4_fn, blob8_ipc4);
alsactl_write(alsa4_fn, blob8_ipc4);

rmpath ../common

end
