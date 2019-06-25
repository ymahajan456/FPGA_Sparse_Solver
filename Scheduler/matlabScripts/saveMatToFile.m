function saveMatToFile(fileName, mat)
    [row, col, val] = find(mat);
	sz = size(mat);
	nnz = length(row);
	fID = fopen(fileName, 'w');
	fwrite(fID, nnz, 'int');
	fwrite(fID, sz(1), 'int');
	fwrite(fID, sz(2), 'int');
	fprintf('%s: Rows: %d, Columns: %d, NNZ: %d\n', fileName, sz(1), sz(2), nnz);
	for i = 1:nnz
		% fprintf(fID, '%d %d %.5g\n', row(i), col(i), val(i));
		fwrite(fID, row(i), 'int');
		fwrite(fID, col(i), 'int');
		fwrite(fID, val(i), 'double');
	end
	fclose(fID);
end
