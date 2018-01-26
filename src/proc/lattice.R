snake2qr <- function(snake) {
	q = snake %% 50
	r = snake %/% 50
	return(c(q,r))
}

center <- function(snake) {
	size=1/sqrt(3)
	offset = c(sqrt(3)/2*size, size)
	height = size * 2
	width = sqrt(3) / 2 * height
	vertical = 0.75 * height
	horiz = width
	qr = snake2qr(snake)
	odd = qr[2] %% 2
	x = offset[1] + qr[1]*horiz + odd*width/2
	y = offset[2] + qr[2]*vertical
	return(c(x, y))
}

vertices <- function(snake) {
	size=1/sqrt(3)
	offset = c(sqrt(3)/2*size, size)
	height = size * 2
	width = sqrt(3) / 2 * height
	xy = center(snake)
	twelve = c( xy[1]          , xy[2] + height/2 )
	two    = c( xy[1] + width/2, xy[2] + height/4 )
	four   = c( xy[1] + width/2, xy[2] - height/4 )
	six    = c( xy[1]          , xy[2] - height/2 )
	eight  = c( xy[1] - width/2, xy[2] - height/4 )
	ten    = c( xy[1] - width/2, xy[2] + height/4 )
	x = c(twelve[1], two[1], four[1], six[1], eight[1], ten[1])
	y = c(twelve[2], two[2], four[2], six[2], eight[2], ten[2])
	return(list('x'=x, 'y'=y))
}

getEndCanopy <- function(path, rid, snake=TRUE) {
	output = c()
	file = read.table(paste(path, '/fruit.', rid, sep=''))
	if(snake) { output=file$V1[file$V3==1] }	
	else { output=file$V2[file$V3==1] }
	return(unique(output))
}

getRNG <- function(path, rid) {
	output=read.table(paste(path, '/RNG.', rid, sep=''))
	names(output) = c('i', 'j', 'w')
	return(output)
}

plotLand <- function(canopy, net=NULL) {
	size = 1/sqrt(3)
	offset = c(sqrt(3)/2*size, size)
	height = size * 2
	width = sqrt(3) / 2 * height
	vertical = 0.75 * height
	horiz = width
	ncol = 50; nrow=50
	xlim=c(0, offset[1] + ncol*width)
	ylim=c(0, offset[2] + (nrow-1)*vertical + height/2)
	plot.new()
	plot.window(xlim, ylim)
	if(!is.null(net)) {
		for(row in 1:nrow(net)) {
			xy_i = center(net$i[row])
			xy_j = center(net$j[row])
			segments(x0=xy_i[1], y0=xy_i[2], x1=xy_j[1], y1=xy_j[2], col='blue')
		}
	}
	for(snake in canopy) { polygon(vertices(snake), col='black') }
}
