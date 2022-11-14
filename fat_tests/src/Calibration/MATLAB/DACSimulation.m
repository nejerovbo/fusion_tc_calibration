% DACSimulation.m
% This script simulates the 5-point, piecewise-linear DDI analog I/O calibration using 
%  a calibration dataset stored in the data structure, <calibrationData>
%  and then computes the verification residuals using a dataset stored in
%  the data structure <verificationData>.
% The piecewise calibration data uses the mean of ten (10) signed DAC values at each
% of five (5) values of calibrationData.MultiMeter==[10 5 0 -5 10].  A
% calibration table comprising [meanSignedDAC(calibrationDataIndices) ...
%   calibrationData.Multimeter(calibrationDataIndices)].
% The host table lookup is then displayed and written to
% "dac_model_output.csv"

DAC_VERSION="1.0.0";

format long

% Define the number of calibration points
powerSourceCalibrationIndices=[10 9 8 7 6 5 4 3 2 1 0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -10];
% numberOfPointsInTheCalibration=size(powerSourceCalibrationIndices,2);
numberOfPointsInTheCalibration = 6400
voltsToMV=1000;  % Set the plotting scale factor for the residuals
numberOfInterpolationPoints=1000;  % This is the number of points in the voltage table lookup representation

% Load the calibration and verification tables.
calibrationData=readtable('cal_point_log.csv');

% numberOfPointsInTheCalibration=calibrationData.

% For now, make the verification data equal to the calibration data
verificationData=calibrationData;

%%%%
% Determine the calibration indices used by the 5-Point calibration
% Loop over the number of points in the calibration to find the indices
calibrationIndices=zeros(1,numberOfPointsInTheCalibration);  %Dimension the calibration
    %indices vector
for iC=1:numberOfPointsInTheCalibration
    calibrationIndices(iC)=calibrationData.MultiMeter(iC);
end
% Now create data tables for calibration and verification data
% Build the matrix of raw calibration ADC values
calibration.Data=[calibrationData.Data1];
sortedCalData=sort(calibration.Data);
sortedCalData = sortedCalData + 32768;
calibration.Data=unsignedToSigned(sortedCalData);  %Convert it to the signed integer format
% Build the matrix of raw verification values
% calibration.Data=[verificationData.Data1];
% calibration.Data=unsignedToSigned(calibration.Data);  %Convert to the signed integer format
%  Now compute the mean value of the calibration ADC at each of the
%  calibration points
%  x
sortedPowerSource=sort(calibrationData.MultiMeter);
voltageAtCalibration=sortedPowerSource;  %This is the voltage 
    %reported by the multimeter at each of the calibration points
adcAtCalibration=calibration.Data; %mean(calibration.Data(calibrationIndices,:),2);  %Get the mean value of the ADC
    %counts at each of the calibration points.
% Now determine the verification residuals
% Get the number of voltage settings in the verification dataset

numberOfVerificationVoltages=size(calibration.Data,1);
numberOfVerificationADC=size(calibration.Data,2);
vHat=zeros(numberOfVerificationVoltages,numberOfVerificationADC);
estimateResidual=zeros(numberOfVerificationVoltages,numberOfVerificationADC);
% Create a figure to plot the piecewise linear voltage lookup
plotDACInterpolation % Plot DAC to host interpolation

% Perform a quantized DAC verification
% quantizedDACVerification

