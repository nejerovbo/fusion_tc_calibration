%plotDACInterpolation.m
% This script transforms the "analog", lookup table (actual voltage
% vs.ADC counts) into a scaled and quantized, host lookup table.  Here,
% "host" is interpreted to mean the EtherCAT Host representation in which
% the voltage range [-10 volts to 10 volts] is mapped into the signed, 16
% bit host representation [-32768 to 32767] range.
% In this, 32768 host counts scale to exactly 10 volts.  So, the maximum
% representable voltage to the host is 9.9998 volts.
% We will first develop the quantized lookup table.

%  (raw DAC code - 0 to 0xFFFF, unsigned)
%       ^          (+10 V, 0xFFFF)
%       |
%       |
%<---------------> X = (signed count)
%       |
%       |
%       |
%       -          (-10 V, 0)

dacEstimateResidual=zeros(1,numberOfVerificationVoltages);

minimumHostCount=-32768; maximumHostCount=32767;  %Set the minimum and maximum counts
hostTableLookupDACInputVector=minimumHostCount:maximumHostCount;  %Create the integer indices that serve as
% input to the table lookup.
maximumHostVoltage=10.;  %Set the maximum voltage
dacAtCalibration = adcAtCalibration; % Set the DAC vectors equal to the ADC vectors for now

voltagePerHostBit=maximumHostVoltage/2^15;
% Now scale the voltageAtCalibration array into the equivalent integer host
% voltage representation
hostVoltageRepresentationAtCalibration=round(voltageAtCalibration/(10/32768));

% Now create the quantized host voltage vector
hostTableLookupVoltageOutputVector=round(interp1(hostVoltageRepresentationAtCalibration,dacAtCalibration,hostTableLookupDACInputVector, "linear"));
% Remove signed component from results for now
hostTableLookupVoltageOutputVector=hostTableLookupVoltageOutputVector + 32768;

% For comparison against the test harness, log vector to CSV
fileID = fopen("dac_model_output.csv","w");
hostCount = maximumHostCount - minimumHostCount;
for count=1:hostCount
    fprintf(fileID,"%d\n",hostTableLookupVoltageOutputVector(count));
end

calibrationDescriptor = 'tmp';
% hostTableLookupVoltageOutputVector=min(hostTableLookupVoltageOutputVector,maximumADCCount);
% hostTableLookupVoltageOutputVector=max(hostTableLookupVoltageOutputVector,minimumADCCount);
% Plot the host voltage lookup table
figure('Name',[datestr(datetime,'mm/dd/yy') ' ' calibrationDescriptor ' Host Voltage Lookup Table'])
plot(hostTableLookupVoltageOutputVector,hostTableLookupDACInputVector,'.',...
    'DisplayName','Quantized Host Voltage Representation')
ylabel ('Host Signed-Integer Voltage Representation (counts)')
xlabel ('DAC Count Index (DAC counts)')
title ('DAC-to-Host-Voltage Table Lookup')
grid on
grid minor
hold on

% Under development - capability to derive the residual error from the
% interpolated values

% Convert back to signed
% hostTableLookupVoltageOutputVector=hostTableLookupVoltageOutputVector + 32768;

%verificationDataLocal = verificationData;
%vhatLocal = zeros(numberOfVerificationVoltages);

%dacAtCalibration=mean(calibration.Data(calibrationIndices,:),2);  %Get the mean value of the DAC
%for iV=1:numberOfVerificationVoltages
%    vhatLocal(iV)=interp1(dacAtCalibration,voltageAtCalibration,calibration.Data(iV),...
%        "linear","extrap" );
%    dacEstimateResidual(iV)=verificationData.MultiMeter(iV) - vhatLocal(iV);
%end

% dacEstimateResidual = voltsToMV * dacEstimateResidual;

% Display the DAC estimate residual
% normplot (dacEstimateResidual);

