params = commandArgs(TRUE)[1]
root = substr(params, 1, nchar(params)-1)
source(params)

aloc = as.matrix(read.csv(paste(root, 'a', sep = ''), header = FALSE))

# Paths
#alocT = t(aloc)
#write.table(alocT, 'res/paths.csv', append = FALSE, quote = FALSE, sep = ',', row.names = FALSE, col.names = FALSE)

# Annual Visits
#for(year in 1:9) {
#	visits = rep(0, n*m)
#	locs = as.vector(t(aloc[((year-1)*baby*mama+1):(year*baby*mama),]))
#	for(i in locs) {visits[i+1] = visits[i+1] + 1}
#	write(visits, 'res/visits.csv', append = TRUE, sep = ',', ncolumns = n*m)
#}

# All Visits
flat = as.vector(aloc)
visits = rep(0, n*m)
for(i in flat) {visits[i+1] = visits[i+1] + 1}
write(visits, paste(root, 'v', sep = ''), append = TRUE, sep = ',', ncolumns = n*m)



