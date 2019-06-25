clear; clc; close;

if ~ exist('../test/exported', 'dir')
    mkdir('../test/exported');
end

files = dir('../test/*.mat');
for file = 1:length(files)
    % load Matrix from file
	fileName = files(file).name;
    fprintf('Processing matrix %s\n', fileName);
	structure = load(strcat('../test/',fileName));
    A = structure.Problem.A;
    %B = structure.Problem.b;
    clear structure;
    % AMD ordering and LU decomposition
    Pamd = amd(A);
    C = A(Pamd, Pamd);
    [Lc, Uc, Pc] = lu(C);
    % [La, Ua, Pa] = lu(A);
    ALUc = Pc * C;
    % ALUa = La * Ua;
    % Store matrices to file
    folderLoc = strcat('../test/exported/', fileName(1:end-4));
    mkdir(folderLoc);
    folderLoc = strcat(folderLoc, '/', fileName(1:end-4));
	saveMatToFile(strcat(folderLoc, '_amd_A.sp'), C);
    %saveMatToFile(strcat(folderLoc, '_B.sp'), B);
    saveMatToFile(strcat(folderLoc, '_amd_L.sp'), Lc);
    saveMatToFile(strcat(folderLoc, '_amd_U.sp'), Uc);
    saveMatToFile(strcat(folderLoc, '_amd_P.sp'), Pc);
    saveMatToFile(strcat(folderLoc, '_amd_Pamd.sp'), Pamd);
    saveMatToFile(strcat(folderLoc, '_amd_ALU.sp'), ALUc);

    % saveMatToFile(strcat(folderLoc, '_A.sp'), A);
    % %saveMatToFile(strcat(folderLoc, '_B.sp'), B);
    % saveMatToFile(strcat(folderLoc, '_L.sp'), La);
    % saveMatToFile(strcat(folderLoc, '_U.sp'), Ua);
    % saveMatToFile(strcat(folderLoc, '_P.sp'), Pa);
    % saveMatToFile(strcat(folderLoc, '_ALU.sp'), ALUa);
end

clear; clc; close;
