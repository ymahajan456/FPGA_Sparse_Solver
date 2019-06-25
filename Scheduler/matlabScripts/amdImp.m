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
    [La, Ua, Pa] = lu(A);
    ALUc = Pc * C;
    ALUa = La * Ua;

    figure;
    subplot(2,2,1);
    spy(ALUa);
    title('Original Matrix A');
    subplot(2,2,2);
    spy(ALUc);
    title('AMD ordered A');
    subplot(2,2,3); 
    spy(La);
    title('L Factor of A');
    subplot(2,2,4); 
    spy(Lc);
    title('L Factor of AMD ordered A');
    fprintf('Processing matrix %s\n', fileName);
end

