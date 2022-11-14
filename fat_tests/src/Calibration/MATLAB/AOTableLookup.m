% AOTableLookup.m
% This script will perform an interpolation from the DAC code domain to
% the host code domain.  It will generate a host-based lookup table ranging
% From -32768 to 32767
% The input data should be formatted as the following
% Channel, MultiMeter, Data1
% Any arbitrary number of channels and points are supported
% The output of the host-based lookup table will be set to "dac_model_output.csv"

AO_LOOKUP_TABLE_GENERATION_VERSION="1.0.1";

% Set MATLAB to read the table in double precision
format long

% numberOfPointsInTheCalibration=size(powerSourceCalibrationIndices,2);
% numberOfPointsInTheCalibration = 800;
voltsToMV=1000;  % Set the plotting scale factor for the residuals
numberOfInterpolationPoints=1000;  % This is the number of points in the voltage table lookup representation

% Load the AO Calibration Table
csvfiles = dir('input/*aout_log*.csv');
for calibrationFile = csvfiles'
    filename = string(calibrationFile.folder) + '/' + string(calibrationFile.name)

    calibrationData = readtable(filename);

    % Determine the maximum number of channels supported
    MaxChannels = max(calibrationData.Channel);

    % Maximum range of the host domain
    MaxHostCode = 65536;

    % Determine the max channel entries
    MaxChannelEntries = size(calibrationData.Channel,2);

    %
    numberOfPointsInTheCalibration = round(MaxChannelEntries/MaxChannels);
    CalibrationDataCh = zeros(numberOfPointsInTheCalibration,MaxChannels);
    hostTableLookupVoltageOutputVector=zeros(MaxHostCode,MaxChannels);

    hostVoltageRepresentationAtCalibrationMulti=zeros(numberOfPointsInTheCalibration,MaxChannels);
    % numberOfPointsInTheCalibration = min(size(voltageAtCalibration,1),numberOfPointsInTheCalibration);

    % channelCount = 0;
    for channelCount = 1:MaxChannels+1
        % Filter the calibration data by channel
        channelFilter = calibrationData.Channel == channelCount-1;
        calibrationDataFiltered = calibrationData(channelFilter,:);

        % Now create data tables for calibration and verification data
        % Build the matrix of raw calibration DAC values
        calibration.Data=[calibrationDataFiltered.Data1];
        sortedCalData=sort(calibration.Data);

        % TODO: Are both of these necessary?
        sortedCalData = sortedCalData + 32768;

        % Convert it to the signed integer format
        calibration.Data=unsignedToSigned(sortedCalData);

        sortedMultiMeter=sort(calibrationDataFiltered.MultiMeter);

        % This is the voltage reported by the multimeter at each of the calibration points
        voltageAtCalibration=sortedMultiMeter;

        % mean(calibration.Data(calibrationIndices,:),2);  %Get the mean value of the ADC
        dacAtCalibration=calibration.Data;

        % Generate the DAC Table-based lookup
        genDACTableLookup
    end

    % Write the host table lookup output to a CSV file
    writematrix(hostTableLookupVoltageOutputVector,string(calibrationFile.folder) + '/../output/' +  string(calibrationFile.name) + "_xor_cal_data.csv");

    % Write out the binary file in unsigned 16-bit representation
    twoMB = zeros(65536,16);
    fileID = fopen(string(calibrationFile.folder) + '/../output/' +  string(calibrationFile.name) + "_xor_cal_data.bin",'w');
    twoMB(1:65536,1:MaxChannels+1) = hostTableLookupVoltageOutputVector;
    for channel = 1:MaxChannels+1
        for count = 1:65536
          % MCM expects big endian
          fwrite(fileID,twoMB(count,channel),'uint16','b');
        end
    end
end
