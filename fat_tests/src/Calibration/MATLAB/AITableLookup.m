% AOTableLookup.m
% This script will perform an interpolation from the ADC code domain to
% the host code domain.  It will generate a host-based lookup table ranging
% From -32768 to 32767
% The input data should be formatted as the following
% Channel, MultiMeter, Data1...Data40
% Any arbitrary number of channels and points are supported
% The output of the host-based lookup table will be set to "dac_model_output.csv"

AI_LOOKUP_TABLE_GENERATION_VERSION="1.0.0";

% Set MATLAB to read the table in double precision
format long

voltsToMV=1000;  % Set the plotting scale factor for the residuals
numberOfInterpolationPoints=1000;  % This is the number of points in the voltage table lookup representation

csvfiles = dir('input/*ain_log*.csv')
csvfiles.name
for calibrationFile = csvfiles'
    filename = string(calibrationFile.folder) + '/' + string(calibrationFile.name)
    calibrationData = readtable(filename);
    % Determine the maximum number of channels supported
    MaxChannels = max(calibrationData.Channel) + 1;

    % Maximum range of the host domain
    MaxHostCode = 65536;

    % Determine the max channel entries

    numberOfPointsInTheCalibration = round(length(calibrationData.MultiMeter)/MaxChannels);

    % Create an output vector to store the table-based lookup
    hostTableLookupVoltageOutputVector=zeros(MaxHostCode,MaxChannels);

    hostVoltageRepresentationAtCalibrationMulti=zeros(numberOfPointsInTheCalibration,MaxChannels);

    for channelCount = 1:MaxChannels
        % Filter the calibration data by channel
        channelFilter = calibrationData.Channel == channelCount-1;
        calibrationDataFiltered = calibrationData(channelFilter,:);

        % Calculate an average of N calibration samples
        vars=calibrationDataFiltered(:,"Data" + wildcardPattern);
        vars=vars{:,:};
        varsSigned = unsignedToSigned(vars);
        AveragedData = mean(varsSigned,2);

        % Convert it to the signed integer format
        calibration.Data=unsignedToSigned(AveragedData);

        % Remove the + V to - V bumping during calibration
        calibration.Data=sort(calibration.Data)

        % Remove the + V to - V bumping during calibration
        sortedMultiMeter=sort(calibrationDataFiltered.MultiMeter);

        % This is the voltage reported by the multimeter at each of the calibration points
        voltageAtCalibration=sortedMultiMeter;

        % mean(calibration.Data(calibrationIndices,:),2);  %Get the mean value of the ADC
        ADCAtCalibration=calibration.Data;

        % Generate the DAC Table-based lookup
        genAITableLookup
    end

    % Write the host table lookup output to a CSV file
    writematrix(hostTableLookupVoltageOutputVector,string(calibrationFile.folder) + '/../output/' +  string(calibrationFile.name) + "_xor_cal_data.csv");

    % Write out the binary file in unsigned 16-bit representation
    twoMB = zeros(65536,16);
    fileID = fopen(string(calibrationFile.folder) + '/../output/' +  string(calibrationFile.name) + "_xor_cal_data.bin",'w');
    twoMB(1:65536,1:MaxChannels) = hostTableLookupVoltageOutputVector;
    for channel = 1:MaxChannels
        for count = 1:65536
          % MCM expects big endian
          fwrite(fileID,twoMB(count,channel),'uint16','b');
        end
    end
end