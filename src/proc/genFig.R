##################################################################
########################    FINAL    #############################
##################################################################
path='/home/colin/projects/SEED/int'
rid='squares_large'
jpeg(filename=paste(path, '/', rid, '.jpg', sep=''))
plotLand(getEndCanopy(path, rid), getRNG(path, rid))
dev.off()

library(spatial)
source('lattice.R')

rid='overdispersed_large'
canopy = getEndCanopy(path, rid)
data <- ppinit(paste(path, '/pp.', rid, '.dat', sep=''))
K = Kfn(data, 35)
plot(K$x, smooth(K$y-K$x), type='l', ylim=c(-4,7))
rids=c('overdispersed_medium', 'reticulated_large', 'reticulated_medium')
for(i in 1:3) {
	canopy = getEndCanopy(path, rids[i])
	data <- ppinit(paste(path, '/pp.', rids[i], '.dat', sep=''))
	K = Kfn(data, 35)
	lines(K$x, smooth(K$y-K$x), col=i+1)
}
###############################################
library(rpg)
library(ggplot2)
library(RColorBrewer)
library(ppcor)
connect("SEED")
SPAR=1
# SMOOTH RETICULATION TIME SERIES
retic_raw = fetch("SELECT rid, year, (rng_diam-rng_nyc)::DOUBLE PRECISION/dmax AS retic FROM graph WHERE rid>1035 ORDER BY rid, year")
retic_raw$rid=as.factor(retic_raw$rid)
retic_smooth = retic_raw
for(RID in levels(retic_smooth$rid)) {
	model = with(retic_smooth[retic_smooth$rid==RID,], smooth.spline(x=year, y=retic, spar=1))
	retic_smooth$retic[retic_smooth$rid==RID] <- predict(model, 0:500)$y
}
write.csv(retic_smooth, '/home/colin/projects/SEED/int/retic_smooth_SPAR1.csv')

#

# TAB:PCOR
ends_raw = fetch("SELECT lag, aspan, (rng_diam-rng_nyc)::DOUBLE PRECISION/dmax AS retic FROM runs, graph WHERE runs.rid>1035 AND runs.rid=graph.rid AND year=500 ORDER BY runs.rid")
ends_smooth = fetch("SELECT lag, aspan, smooth FROM runs, graph WHERE runs.rid>1035 AND runs.rid=graph.rid AND year=500 ORDER BY runs.rid")
peak = fetch("SELECT lag, aspan, year, smooth FROM runs, (SELECT rid, year, smooth, MAX(smooth) OVER (PARTITION BY rid) FROM graph WHERE rid>1035) AS foo WHERE runs.rid=foo.rid AND smooth=max")

data = fetch("SELECT lag, aspan, foo.year AS tmax, foo.smooth AS Rmax, graph.smooth AS R500 FROM graph, runs, (SELECT rid, year, smooth, MAX(smooth) OVER (PARTITION BY rid) FROM graph WHERE rid>1035) AS foo WHERE runs.rid=foo.rid AND runs.rid=graph.rid AND foo.smooth=foo.max AND graph.year=500 ORDER BY lag, aspan")

# PCOR
THR=11
# tmax
tmax_lg = pcor(data[data$lag < THR,1:3], 'spearman')$estimate[1:2,3]
tmax_hg = pcor(data[data$lag > THR,1:3], 'spearman')$estimate[1:2,3]
#pcor(data[,1:3], 'spearman')$estimate
# R'max
Rmax_lg = pcor(data[data$lag < THR,c(1,2,4)], 'spearman')$estimate[1:2,3]
Rmax_hg = pcor(data[data$lag > THR,c(1,2,4)], 'spearman')$estimate[1:2,3]
#pcor(data[,c(1,2,4)], 'spearman')$estimate
# R'(500)
R500_lg = pcor(data[data$lag < THR,c(1,2,5)], 'spearman')$estimate[1:2,3]
R500_hg = pcor(data[data$lag > THR,c(1,2,5)], 'spearman')$estimate[1:2,3]
#pcor(data[,c(1,2,5)], 'spearman')$estimate
round(cbind(R500_lg, tmax_lg, Rmax_lg, R500_hg, tmax_hg, Rmax_hg), 4)


# tmax, R'max
pcor(data[data$lag<=12,1:4], 'spearman')$estimate
pcor(data[data$lag>12,1:4], 'spearman')$estimate
pcor(data[,1:4], 'spearman')$estimate
# tmax, R'(500)
pcor(data[data$lag<=12,c(1,2,3,5)], 'spearman')$estimate
pcor(data[data$lag>12,c(1,2,3,5)], 'spearman')$estimate
pcor(data[,c(1,2,3,5)], 'spearman')$estimate
# R'max, R'(500)
pcor(data[data$lag<=12,c(1,2,4,5)], 'spearman')$estimate
pcor(data[data$lag>12,c(1,2,4,5)], 'spearman')$estimate
pcor(data[,c(1,2,4,5)], 'spearman')$estimate

# tmax, R'max, R'(500)
pcor(data[data$lag<=12,], 'spearman')$estimate
pcor(data[data$lag>12,], 'spearman')$estimate
pcor(data), 'spearman')

# FIG:RETIC
library(ggplot2)
source('/home/colin/projects/SEED/src/final/lattice.R')

RNG = read.table('/home/colin/projects/SEED/int/RNG.clustered', sep=' ')
canopy = unique(c(RNG$V1, RNG$V2))
i = sapply(RNG[,1], center)
j = sapply(RNG[,2], center)

par(mar=c(0,0,0,0), oma=c(0,0,0,0))
plot(NULL, NULL, xlim=c(min(vertices(0)[,1]), max(vertices(2499)[,1])), ylim=c(min(vertices(0)[,2]), max(vertices(2499)[,2])), axes=FALSE, ann=FALSE)
segments(x0=i[1,], y0=i[2,], x1=j[1,], y1=j[2,], col='blue')
for (snake in canopy) {polygon(vertices(snake), col=grey(0.2), border=grey(0.2))}
box()

# FIG:EXAMPLE

# FIG:EXTENT:REG
library(rpg)
library(ggplot2)
library(RColorBrewer)
connect("SEED")

data = fetch("SELECT lag, aspan, dmax FROM runs, graph WHERE runs.rid>1035 AND runs.rid=graph.rid AND year=500 ORDER BY lag, aspan")
reg = lm(dmax~lag+aspan, data=data)
data = fetch("SELECT lag, aspan, AVG(dmax)::DOUBLE PRECISION AS dmax, COUNT(dmax) FROM runs, graph WHERE runs.rid>1035 AND runs.rid=graph.rid AND year=500 GROUP BY lag,aspan ORDER BY lag, aspan")

rescale_lag = c(seq(0,0.65,length.out=10), seq(0.65, 1, length.out=10)[2:10])

label_y = 'Mean Final Extent'
label_x = 'Tree Lifespan (yrs.)'
label_lag="Gut Retent.\nTime (steps)"

pdf(file='/home/colin/Dropbox/Apps/ShareLaTeX/mydiss/fig/SEED-extent.pdf', onefile=FALSE, width=6, height=5, pointsize=12)

ggplot(data) + 
 geom_abline(slope=reg$coefficients[3], intercept=reg$coefficients[1]+reg$coefficients[2]*2, color='#2166AC', lty='dashed') +

 geom_abline(slope=reg$coefficients[3], intercept=reg$coefficients[1]+reg$coefficients[2]*20, color='#B2182B', linetype='dashed') +

 geom_point(aes(x=aspan, y=dmax, color=lag)) +

 scale_y_continuous(name=label_y) + scale_x_continuous(name=label_x) +

 #scale_colour_gradient2(label_lag, low = "#67001F", mid = "#F7F7F7", high ="#053061", midpoint = 13, guide = "colourbar") +
 #scale_color_manual(name=label_lag, breaks=seq(2,20,2), values=grad_lag) +
 scale_color_distiller(label_lag, palette='RdBu', values=rescale_lag, breaks=c(2,13,20), labels=c(2,13,20)) + 
 
 theme_minimal() + theme(panel.grid.major=element_blank(),
  panel.grid.minor=element_blank(), plot.margin=unit(c(0,0,0,0),
  units="lines"))

dev.off()

# FIG:PEAKS
library(rpg)
library(ggplot2)
library(RColorBrewer)
library(grid)
connect("seed")

data = fetch("SELECT lag, aspan, AVG(year) AS year, AVG(smooth) AS smooth FROM runs, (SELECT rid, year, smooth, MAX(smooth) OVER (PARTITION BY rid) FROM graph WHERE rid>1035) AS foo WHERE runs.rid=foo.rid AND smooth=max GROUP BY lag, aspan ORDER BY lag DESC, aspan DESC")


data$year = as.integer(data$year)
label_lag='g'
label_aspan=expression(tau[A])
label_Rmax = 'Peak Reticulation'
label_tmax = 'Peak Year'
##### VERSION 1 #####
data$aspan=as.factor(data$aspan)
ggplot(data) + geom_point(aes(y=year, x=lag)) + facet_wrap(~aspan, ncol=1)

##### VERSION 2 #####
data$aspan=as.factor(data$aspan)
ggplot(data) + geom_point(aes(y=smooth, x=aspan)) + facet_wrap(~lag, ncol=1)

##### VERSION 3 #####
data = fetch("SELECT lag, aspan, year, smooth FROM runs, (SELECT rid, year, smooth, MAX(smooth) OVER (PARTITION BY rid) FROM graph WHERE rid>1035) AS foo WHERE runs.rid=foo.rid AND foo.smooth=max ORDER BY lag DESC, aspan DESC")
data$year = as.integer(data$year)
data$lag = as.factor(data$lag)
data$aspan=as.factor(data$aspan)
ggplot(data) + geom_point(aes(y=smooth, x=year)) + facet_grid(lag~aspan)



rescale_lag = c(seq(0,0.65,length.out=10), seq(0.65, 1, length.out=10)[2:10])
size=3

#ggplot(data) + geom_point(shape=21, aes(y=smooth, x=year, fill=aspan, size=lag, stroke=0)) + scale_size(name=label_lag, range=c(3,12)) + scale_fill_distiller(palette='BrBG') + scale_y_continuous(name='Peak Year') + scale_x_continuous(name='Peak Reticulation') + theme_minimal() + theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank())

#offset=2; 
#ggplot(data) + geom_point(aes(x=year-offset, y=smooth, color=lag), shape="\u25D6", size=size) + geom_point(aes(x=year+offset, y=smooth, color=aspan), shape="\u25D7", size=size)

pdf(file='/home/colin/Dropbox/Work/mydiss/fig/SEED-peaks.pdf', onefile=FALSE, width=6, height=5, pointsize=12)

top_plot=ggplot(data) + theme_minimal() + 
geom_point(shape=21, aes(y=smooth, x=year, fill=lag, stroke=0), size=size) +
scale_fill_distiller(label_lag, palette='RdBu', values=rescale_lag, breaks=c(2,13,20), labels=c(2,13,20)) +
scale_y_continuous(name=label_y) +
theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank(),
 axis.title.x=element_blank(), axis.text.x=element_blank(), axis.ticks.x=element_blank(),
 plot.margin=unit(c(0,0,0,0), units="lines"))

bottom_plot=ggplot(data) + theme_minimal() + 
geom_point(shape=21, aes(y=smooth, x=year, fill=aspan, stroke=0), size=size) +
scale_fill_distiller(name=label_aspan, palette='BrBG') +
scale_y_continuous(name=label_y) + scale_x_continuous(name=label_x) +
theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank(),
 plot.margin=unit(c(0,0,0,0), units="lines"))

grid.newpage()
grid.draw(rbind(ggplotGrob(top_plot), ggplotGrob(bottom_plot), size = "last"))
dev.off()

# FIG:YEAR
data=fetch("SELECT year, (rng_diam-rng_nyc)::DOUBLE PRECISION/dmax AS retic FROM graph WHERE rid=1502")
data=cbind(data, 'smooth'=predict(smooth.spline(x=data$year, y=data$retic, spar=SPAR), 0:500)$y)
ggplot(retic_smooth) + geom_line(aes(x=year, y=retic, group=rid))
