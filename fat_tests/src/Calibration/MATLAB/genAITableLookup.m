% genAITableLookup.m
% This script transforms the "analog", lookup table (actual voltage
% vs.ADC counts) into a scaled and quantized, host lookup table.  Here,
% "host" is interpreted to mean the EtherCAT Host representation in which
% the voltage range [-10 volts to 10 volts] is mapped into the signed, 16
% bit host representation [-32768 to 32767] range.
% In this, 32768 host counts scale to exactly 10 volts.  So, the maximum
% representable voltage to the host is 9.9998 volts.
% We will first develop the quantized lookup table.

%  (raw ADC code - 0x7FFF to 0x8000, signed)
%       ^          (+10 V, 0x7FFF)
%       |
%       |
%<---------------> X = (signed count)
%       |
%       |
%       |
%       -          (-10 V, 0x8000)

minimumADCCount=-32768; maximumADCCount=32767;  %Set the minimum and maximum host counts

% Create the integer indices that serve as input to the table lookup.
hostTableLookupADCInputVector=minimumADCCount:maximumADCCount;

% Set the maximum voltage
maximumHostVoltage=10.;

% Set the scaling per bit
voltagePerHostBit=maximumHostVoltage/2^15;

numberOfPointsInTheCalibration = size(voltageAtCalibration,1);
numCalPoints = 1:numberOfPointsInTheCalibration;

% Now scale the voltageAtCalibration array into the equivalent integer host voltage representation
hostVoltageRepresentationAtCalibrationMulti(numCalPoints,channelCount)=round(voltageAtCalibration(numCalPoints)/(10/32768));

% Now create the quantized host voltage vector
hostTableLookupVoltageOutputVector(:,channelCount)=round(interp1(ADCAtCalibration(numCalPoints),...
    hostVoltageRepresentationAtCalibrationMulti(numCalPoints,channelCount),hostTableLookupADCInputVector, "linear", "extrap"));

hostTableLookupVoltageOutputVector(:,channelCount)=min(hostTableLookupVoltageOutputVector(:,channelCount),maximumADCCount);
hostTableLookupVoltageOutputVector(:,channelCount)=max(hostTableLookupVoltageOutputVector(:,channelCount),minimumADCCount);

% Remove signed component from results for now
hostTableLookupVoltageOutputVector(:,channelCount)=hostTableLookupVoltageOutputVector(:,channelCount);

TableLookupOutput=signedToUnsigned(hostTableLookupVoltageOutputVector(:,channelCount));

% Swap the -10 to +10 V data into the two's complement
% 0 V -> +10 V -> -10 V -> 0 V representation
%tmp = index 1
%index 1->i32769
%i32769->tmp
for swapCount = 1:32768
  a = TableLookupOutput(swapCount);
  b = TableLookupOutput(swapCount+32768);
  TableLookupOutput(swapCount) = b;
  TableLookupOutput(swapCount+32768) = a;
end

% Dimension the index array
Index = TableLookupOutput;

% Create a linear index mapping from 0 to 65536, this is used by the 
% DDI XOR algorithm.  XOR value = value ^ index & 0xFFFF;
for indexCount = 1:size(Index)
  Index(indexCount) = indexCount-1;
end

% Represent the 0xFFFF term
hex_ffff = zeros(65536,1) + 65535;

% Perform the XOR operations
xor_output1 = bitxor(Index,TableLookupOutput);
xor_output_final = bitxor(xor_output1,hex_ffff);

hostTableLookupVoltageOutputVector(:,channelCount)=xor_output_final;

% At this point, the script terminates.  The main application should write
% the hostTableLookupVoltageOutputVector to a log file
