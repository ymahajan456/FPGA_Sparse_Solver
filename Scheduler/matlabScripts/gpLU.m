function [L, U, P] = gpLU(A)
    sz = size(A);
    m = sz(1);
    L = eye(m);
    P = [1 2 3 4];
    U = L;
    for j = 1:m
        Lj = L(1:j-1, 1:j-1);
        aj = A(1:j-1,j);
        uj = Lj\aj;
        %.......
        ajd = A(j:m, j);
        Ljd = L(j:m,1:j-1);
        bjd = ajd - Ljd * uj;
        %........
        % [ujj, id] = max(bjd);
        % bjd(id) = bjd(1);
        % bjd(1) = ujj;
        
        % tmp = A(id + j -1,:);
        % A(id + j - 1,:) = A(j,:);
        % A(j,:) = tmp;

        % tmp = P(id + j - 1);
        % P(id + j - 1) = P(j);
        % P(j) = tmp; 
        
        % tmp = L(id + j -1);
        % L(id + j - 1) = L(j);
        % L(j) = tmp;
        
        ujj = bjd(1);
        if(ujj == 0)
           error('Zero diagonal entry');
        end
        L(j:m, j) = bjd/ujj;
        U(j,j) = ujj;
        U(1:j-1, j) = uj;
    end
end