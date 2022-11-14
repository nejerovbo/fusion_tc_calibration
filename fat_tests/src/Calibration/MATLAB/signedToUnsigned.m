function [unsignedMatrix] = signedToUnsigned(signedMatrix)
    %This script converts a matrix of unsigned integers to a matrix of
    %equivalent signed integers
    %Initialization
    signedMax=32767;
    delta=2^16;
    %Get matrix size
    matrixSize=size(signedMatrix);
    unsignedMatrix=signedMatrix;  %Dimension the signed matrix
    %Loop over all elements of the unsignedMatrix and compute the signed
    %equivalent
    for iV1=1:matrixSize(1)
        for iV2=1:matrixSize(2)
            if unsignedMatrix(iV1,iV2)< 0
                unsignedMatrix(iV1,iV2)=unsignedMatrix(iV1,iV2)+65536;
            end
        end
    end
end
