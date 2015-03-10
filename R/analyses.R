# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                     GET.DATA                     *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

get.data <- function (city="nyc", from=TRUE, covar=FALSE, msg=FALSE)
{
    txt.dir <- "../results/"
    fname <- paste (txt.dir, "stationDistsMat_", city, ".csv", sep="")
    dists <- as.matrix (read.csv (fname, header=FALSE))
    dims <- dim (dists)
    dists <- array (dists, dim=dims)
    if (from) txt.ft <- "from"
    else txt.ft <- "to"
    if (covar)
    {
        fname <- paste (txt.dir, "Cov_", txt.ft, "_", city, ".csv", sep="")
        y <- as.matrix (read.csv (fname, header=FALSE))
        indx <- which (dists < 0)
    } else {
        fname <- paste (txt.dir, "R2_", txt.ft, "_", city, ".csv", sep="")
        y <- as.matrix (read.csv (fname, header=FALSE))
        indx <- which (dists < 0 | abs (y) > 1)
    }
    y <- array (y, dim=dims)

    dists [indx] <- NA
    y [indx] <- NA
    # London has some extreme outliers in covariances, so
    if (city == "london")
    {
        indx <- which (y > -min (y, na.rm=TRUE))
        if (msg)
            cat ("London-", txt.ft, ": Removed ", length (indx), " / ", 
                    length (dists), " = ", 
                    formatC (100 * length (indx) / length (dists), 
                    format="f", digits=1), "% extreme covariances.\n", sep="")
        #dists [indx] <- NA
        #y [indx] <- NA
    }

    return (list (d=dists, y=y))
}

# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                     FIT.DECAY                    *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

fit.decay <- function (city="nyc", from=TRUE, mod.type="power", covar=TRUE, 
                       threshold=0, ylims=NULL, plot=TRUE)
{
    msg <- FALSE
    if (plot) msg <- TRUE
    # If !covar, then models are fitted to R2 values, otherwise they are fitted
    # to covariances.
    # "threshold" presumes that decays with intercepts lower than this value
    # (expressed as a fraction 0->1 of the maximal intercept) will not generate
    # informative k-values, and are therefore rejected. However, the
    # "compare.thresholds" function below demonstrates that this is clearly not
    # the case, and rejecting these values leads to increases in overall
    # aggregate errors.
    dat <- get.data(city=city, from=from, covar=covar, msg=msg)

    n <- dim (dat$d)[1]
    index <- kvec <- intercept <- ss <- NULL
    if (from) ftxt <- "from"
    else ftxt <- "to"
    # Note that the FROM data contain 2 junk cases which are readily
    # identifiable from visual inspection. These can be quantified in each of
    # the following three cases through:
    # 1. "power": k > 100
    # 2. "Gaussian": k < 0
    # 3. "logGauss": k > 2
    for (i in 1:n)
    {
        d <- dat$d [i, ]
        y <- dat$y [i, ]
        indx <- which (!is.na (d) & !is.na (y) & y > 0)
        # There are some cases for which trip data exist yet station distances
        # haven't yet been added to list. These produce NAs in the dists table.
        if (length (indx) > 10)
        {
            d <- d[indx]
            y <- y[indx]
            dfit <- seq(min(d), max(d), length.out=100)
            if (mod.type == "power") {
                # y ~ a * d ^ (-k)
                # log y = log a -k log d
                # Gaussian k-values are at y=exp(-1), so equivalent values for
                # power-laws are
                # d^(-k)=e^(-1)
                # d^k = e
                # k log d = 1
                # d = exp (1/k)
                indx <- which (d > 0) # Because 0 distances do occur
                d <- d [indx]
                y <- y [indx]
                mod <- lm (log10 (y) ~ log10 (d))
                coeffs <- summary (mod)$coefficients
                if (exp (-1 / coeffs[2]) < 100)
                {
                    yfit <- 10 ^ predict (mod)
                    ssi <- (yfit - y) ^ 2
                    ss <- c (ss, mean (ssi, na.rm=TRUE))
                    intercept <- c (intercept, 10 ^ coeffs [1])
                    kvec <- c (kvec, exp (-1 / coeffs [2]))
                    index <- c (index, i)
                }
                tstr <- paste (toupper (city), "-", ftxt, "-power-law", sep="")
            } else if (mod.type == "Gaussian") {
                if (covar)
                    a0 <- 2 * mean (y)
                else
                    a0 <- 1
                k0 <- 1
                mod <- NULL
                while (is.null (mod) & k0 < 5)
                {
                    k0 <- k0 + 1
                    mod <- tryCatch (nls (y ~ y0 + a * exp (-d^2 / k^2), 
                                start=list(y0=0, a=2*mean(y),k=2)),
                                error=function(e) NULL)
                }
                if (!is.null (mod))
                {
                    coeffs <- summary (mod)$coefficients
                    if (coeffs [3] > 0)
                    {
                        ss <- c (ss, mean (summary (mod)$residuals ^ 2))
                        kvec <- c (kvec, coeffs [3])
                        intercept <- c (intercept, coeffs [2])
                        index <- c (index, i)
                    }
                }
                tstr <- paste (toupper (city), "-", ftxt, "-Gaussian", sep="")
            } else if (mod.type == "logGauss") {
                k0 <- 0.1
                mod <- NULL
                while (is.null (mod) & k0 < 1)
                {
                    k0 <- k0 + 0.1 
                    mod <- tryCatch (nls (y ~ a * exp (-log10(d)^2 / k^2), 
                                start=list(a=2*mean(y),k=0.1)),
                                error=function(e) NULL)
                }
                if (!is.null (mod))
                {
                    if (summary (mod)$coefficients [2] < 2)
                    {
                        ss <- c (ss, mean (summary (mod)$residuals ^ 2))
                        coeffs <- summary (mod)$coefficients
                        kvec <- c (kvec, 10 ^ coeffs [2])
                        intercept <- c (intercept, coeffs [1])
                        index <- c (index, i)
                    }
                }
                tstr <- paste (toupper (city), "-", ftxt, "-log-Gaussian",
                               sep="")
            }
        } # end if len (indx) > 10
    } # end for i
    #cat ("count = ", count, "\n", sep="")
    # Any cases that generate k-values that are greater than max (d) are very
    # likely spurious, so are removed here
    len.full <- length (ss)
    maxd <- max (dat$d, na.rm=TRUE)
    indx <- which (kvec > maxd)
    if (msg & length (indx) > 0)
        cat (city, "-", mod.type, "-", ftxt, ": Removed ", length (indx), 
             " k-values > d = ", maxd, "km\n", sep="")
    indx <- kvec < maxd
    ss <- ss [indx]
    index <- index [indx]
    kvec <- kvec [indx]
    intercept <- intercept [indx] 
    # And then reject all intercepts < threshold
    if (threshold > 0)
    {
        nout <- length (which (intercept < (threshold * max (intercept))))
        if (msg)
            cat (city, "-", mod.type, "-", ftxt, ": Removed ", 
                nout, " / ", length (intercept), " = ", 
                formatC (100 * nout / length (intercept), format="f", digits=2),
                "% of values with intercepts < ", threshold, " = ", 
                round (threshold * max (intercept)), "\n", sep="")
        indx <- which (intercept > (threshold * max (intercept)))
        ss <- ss [indx]
        index <- index [indx]
        kvec <- kvec [indx]
        intercept <- intercept [indx] 
    }

    if (covar)
        ss <- ss / 1e7
    dat <- data.frame (cbind (index, kvec, intercept, ss)) 
    colnames (dat) <- c("i", "k", "y", "ss")

    if (plot)
    {
        if (length (ylims) < 2) ylims <- range (kvec, na.rm=TRUE)
        plot (intercept, kvec, pch=1, col="orange", bty="l", ylim=ylims)
        mod <- lm (kvec ~ intercept)
        x <- seq (min (intercept), max (intercept), length.out=100)
        y <- predict (mod, new=data.frame(intercept=x))
        lines (x, y, col="red")

        xpos <- min (intercept) + 0.5 * diff (range (intercept))
        ypos <- ylims [1] + c(0.8, 0.6) * diff (ylims)
        par (ps=18)
        text (xpos, ypos[1], labels=tstr)
        par (ps=12)
        r2 <- sign (summary (mod)$coefficients [2]) * summary (mod)$r.squared
        tstr2 <- paste ("R2 = ", formatC (r2, format="f", digits=4), 
                    " (p=", formatC( summary (mod)$coefficients [8],
                    format="f", digits=4), ");\n mean(k) = ", 
                    formatC (mean (dat$k), format="f", digits=2),
                   "+/-", formatC (sd (dat$k), format="f", digits=2),
                   "; ss = ", formatC (mean (dat$ss), format="f", digits=2), 
                   sep="")
        text (xpos, ypos[2], labels=tstr2)
        par (ps=10)
    }
    return (dat)
}

# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                   FIT.GAUSSIAN                   *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

fit.gaussian <- function (city="nyc", from=TRUE, covar=TRUE)
{
    # Produces a data frame with [i, k, y, ss, ntrips]
    # If !covar, then models are fitted to R2 values, otherwise they are fitted
    # to covariances.
    msg <- TRUE
    dat <- get.data (city=city, from=from, covar=covar, msg=msg)
    fname <- paste ("../results/NumTrips_", city, ".csv", sep="")
    ntrips.mat <- read.csv (fname, header=FALSE)

    n <- dim (dat$d)[1]
    if (n != dim (ntrips.mat) [1])
        cat ("ERROR: size of ntrips not same as covariance matrix!\n")
    index <- kvec <- intercept <- ss <- ntrips <- NULL
    if (from) ftxt <- "from"
    else ftxt <- "to"
    # Note that the FROM data contain 2 junk cases with k<0!
    for (i in 1:n)
    {
        d <- dat$d [i, ]
        y <- dat$y [i, ]
        indx <- which (!is.na (d) & !is.na (y) & y > 0)
        # There are some cases for which trip data exist yet station distances
        # haven't yet been added to list. These produce NAs in the dists table.
        if (length (indx) > 10)
        {
            d <- d[indx]
            y <- y[indx]
            dfit <- seq(min(d), max(d), length.out=100)
            if (covar)
                a0 <- 2 * mean (y)
            else
                a0 <- 1
            k0 <- 1
            mod <- NULL
            while (is.null (mod) & k0 < 5)
            {
                k0 <- k0 + 1
                mod <- tryCatch (nls (y ~ y0 + a * exp (-d^2 / k^2), 
                            start=list(y0=0, a=2*mean(y),k=2)),
                            error=function(e) NULL)
            }
            if (!is.null (mod))
            {
                coeffs <- summary (mod)$coefficients
                if (coeffs [3] > 0)
                {
                    ss <- c (ss, mean (summary (mod)$residuals ^ 2))
                    kvec <- c (kvec, coeffs [3])
                    intercept <- c (intercept, coeffs [2])
                    index <- c (index, i)
                    ntrips <- c (ntrips, sum (ntrips.mat [,i]))
                }
            }
            tstr <- paste (toupper (city), "-", ftxt, sep="")
        } # end if len (indx) > 10
    } # end for i
    # Any cases that generate k-values that are greater than max (d) are very
    # likely spurious, so are removed here
    len.full <- length (ss)
    maxd <- max (dat$d, na.rm=TRUE)
    indx <- which (kvec > maxd)
    if (msg & length (indx) > 0)
        cat (city, "-", ftxt, ": Removed ", length (indx), 
             " k-values > d = ", maxd, "km\n", sep="")
    indx <- kvec < maxd
    ss <- ss [indx]
    index <- index [indx]
    kvec <- kvec [indx]
    intercept <- intercept [indx] 
    ntrips <- ntrips [indx]

    if (covar)
        ss <- ss / 1e7
    dat <- data.frame (cbind (index, kvec, intercept, ntrips, ss)) 
    colnames (dat) <- c("i", "k", "y", "ntrips", "ss")
    return (dat)
}

# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                  COMPARE.NTRIPS                  *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

compare.ntrips <- function (covar=TRUE)
{
    cities <- c ("london", "nyc")
    x11 ()
    par (mfrow=c(2,2), mar=c(2.5,2.5,0.5,0.5), mgp=c(1.3,0.7,0), ps=10)
    ft <- c (TRUE, FALSE)
    ftxt <- c ("from", "to")
    for (city in cities)
    {
        for (i in 1:2) {
            dat <- fit.gaussian (city=city, from=ft[i], covar=covar)
            n <- dat$ntrips
            k <- dat$k
            plot (n, k, pch=1, col="orange", bty="l",
                  xlab="ntrips", ylab="k-value")
            mod <- lm (k ~ n)
            x <- seq (min (n), max (n), length.out=100)
            y <- predict (mod, new=data.frame(n=x))
            lines (x, y, col="blue")

            r2 <- summary (mod)$r.squared
            pval <- summary (mod)$coefficients [8]
            xpos <- min (n) + 0.5 * diff (range (n))
            ypos <- min (k) + c(0.8, 0.6, 0.4) * diff (range (k))
            par (ps=18)
            text (xpos, ypos[1], labels=paste (toupper (city), ftxt [i]))
            par (ps=12)
            text (xpos, ypos[2], labels= paste ("r2 = ",
                formatC (r2, format="f", digits=4), " (p=",
                formatC (pval, format="f", digits=4), ")", sep=""))
            par (ps=10)
        }
    }
}

# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                  COMPARE.MODELS                  *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

compare.models <- function (city="nyc", from=TRUE, covar=TRUE, threshold=0)
{
    mod.types <- c ("power", "Gaussian", "logGauss")
    x11 ()
    par (mfrow=c(2,2), mar=c(2.5,2.5,0.5,0.5), mgp=c(1.3,0.7,0), ps=10)
    fulldat <- list()
    for (i in 1:3)
        fulldat [[i]] <- fit.decay(city=city, from=from, mod.type=mod.types[i],
                                   covar=covar, threshold=threshold, plot=TRUE)
    # matrices of correlations and t-tests between k-values
    imax <- max (sapply (fulldat, function (x) max (x$i)))
    kvals <- array (NA, dim=c(imax, 3))
    for (i in 1:3)
        kvals [fulldat[[i]]$i, i] <- fulldat[[i]]$k
    r12 <- cor (kvals[,1], kvals[,2], use="pairwise.complete")
    r13 <- cor (kvals[,1], kvals[,3], use="pairwise.complete")
    r23 <- cor (kvals[,2], kvals[,3], use="pairwise.complete")
    p12 <- t.test (kvals[,1], kvals[,2], paired=TRUE)$p.value
    p13 <- t.test (kvals[,1], kvals[,3], paired=TRUE)$p.value
    p23 <- t.test (kvals[,2], kvals[,3], paired=TRUE)$p.value
    r2 <- c (r12, r13, r23)
    pvals <- c (p12, p13, p23)
    txt <- c ("pow-gau", "pow-log", "gau-log")
    for (i in 1:3)
        cat (txt[i], ": R2 = ", formatC (r2[i], format="f", digits=4),
             "; t-test p = ", formatC (pvals[i], format="f", digits=4),
             "\n", sep="")

    imax <- max (c (max (fulldat[[2]]$i), max (fulldat[[3]]$i)))
    x <- y <- rep (NA, imax)
    x [fulldat[[2]]$i] <- fulldat[[2]]$k
    y [fulldat[[3]]$i] <- fulldat[[3]]$k
    plot (x, y, pch=1, col="lawngreen", bty="l",
          xlab="k Gaussian", ylab="k log-Gaussian")
    mod <- lm (y ~ x)
    xrange <- range (x, na.rm=TRUE)
    xfit <- seq (xrange [1], xrange [2], length.out=100)
    yfit <- predict (mod, new=data.frame(x=xfit))
    lines (xfit, yfit, col="blue")

    xpos <- min (x, na.rm=TRUE) + 0.5 * diff (range (x, na.rm=TRUE))
    ypos <- min (y, na.rm=TRUE) + c(0.8, 0.6, 0.4) * diff (range (y, na.rm=TRUE))
    par (ps=18)
    text (xpos, ypos[1], labels="Gaussian ~ log-Gaussian")
    if (from) text (xpos, ypos[3], labels="movement FROM")
    else text (xpos, ypos[3], labels="movement TO")
    r2 <- cor (x, y, use="pairwise.complete") ^ 2
    pval <- summary (mod)$coefficients[8]
    par (ps=12)
    text (xpos, ypos[2], labels= paste ("r2 = ",
        formatC (r2, format="f", digits=4), " (p=",
        formatC (pval, format="f", digits=4), ")", sep=""))

    ss <- sapply (fulldat, function (x) mean (x$ss))
    kmn <- sapply (fulldat, function (x) mean (x$k))
    ksd <- sapply (fulldat, function (x) sd (x$k))
    dat <- data.frame (cbind (mod.types, kmn, ksd, ss))
    return (dat)
}

# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                COMPARE.THRESHOLDS                *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

compare.thresholds <- function (city="nyc", from=TRUE, covar=TRUE)
{
    # This takes quite some time to execute, and just serves to demonstrate that
    # model fits are better in all cases with thresholds of zero. The whole
    # threshold concept could thus be ditched, but this has been retained just
    # to demonstrate a quantitative reason why.
    mod.types <- c ("power", "Gaussian", "logGauss")
    thresholds <- 0:10 / 100
    arr <- array (NA, dim=c(length (thresholds), length (mod.types)))
    dat <- list (kmn=arr, ss=arr)

    for (i in 1:length (thresholds))
    {
        cat (i, ".", sep="")
        fulldat <- list()
        for (j in 1:3)
            fulldat [[j]] <- fit.decay(city=city, from=from, mod.type=mod.types[j],
                                       covar=covar, threshold=thresholds[i],
                                       plot=FALSE)
        
        dat$ss [i, ] <- sapply (fulldat, function (x) mean (x$ss))
        dat$kmn [i, ]<- sapply (fulldat, function (x) mean (x$k))
    }
    cat ("\n")

    x11 ()
    par (mfrow=c(1,2), mar=c(2.5,2.5,2,2), mgp=c(1.3,0.7,0), ps=10)
    
    cols <- c ("red", "blue", "lawngreen")
    ylabs <- c ("k-mean", "SS")
    if (from) mt <- paste (toupper (city), "from")
    else mt <- paste (toupper (city), "to")
    # plots not looped, because SS has to have separate axes for the mod.types
    ylims <- range (dat$kmn)
    plot (thresholds, dat$kmn [,1], "l", col=cols[1], ylim=ylims, bty="l",
          xlab="Threshold", ylab="k-mean", main=mt)
    for (j in 1:3)
        lines (thresholds, dat$kmn [,j], col=cols[j])
    ypos <- ylims [1] + 0.5 * diff (ylims)
    legend (0, ypos, legend=mod.types, col=cols, lwd=1, bty="n")

    ylims <- range (dat$ss [,2:3])
    plot (thresholds, dat$ss [,2], "l", col=cols[2], ylim=ylims, bty="l",
          xlab="Threshold", ylab="SS")
    for (j in 2:3)
        lines (thresholds, dat$ss [,j], col=cols[j])
    par (new=TRUE)
    plot (thresholds, dat$ss [,1], "l", col=cols[1], 
          xaxt="n", yaxt="n", xlab="", ylab="", frame=FALSE)
    Axis (range (dat$ss [,1]), side=4, col=cols[1])
}

# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                  COMPARE.TOFROM                  *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

compare.tofrom <- function (covar=TRUE, threshold=0)
{
    x11 (width=13.5)
    par (mfrow=c(2,3), mar=c(2.5,2.5,0.5,0.5), mgp=c(1.3,0.7,0), ps=10)

    cities <- c ("nyc", "london")
    for (city in cities)
    {
        to <- fit.decay(city=city, from=FALSE, "Gaussian", covar=covar, 
                            ylims=c(0, 10), threshold=threshold)
        from <- fit.decay(city=city, from=TRUE, "Gaussian", covar=covar, 
                            ylims=c(0, 10), threshold=threshold)
        # matrices of correlations and t-tests between k-values
        imax <- max (c (max (to$i), max (from$i)))
        x <- y <- rep (NA, imax)
        x [to$i] <- to$k
        y [from$i] <- from$k

        xylims <- c (0, min (c (max (x, na.rm=TRUE), max (y, na.rm=TRUE))))
        plot (x, y, pch=1, col="lawngreen", bty="l", xlim=xylims, ylim=xylims,
              xlab="k Gaussian", ylab="k log-Gaussian")
        mod <- lm (y ~ x)
        xrange <- range (x, na.rm=TRUE)
        xfit <- seq (xrange [1], xrange [2], length.out=100)
        yfit <- predict (mod, new=data.frame(x=xfit))
        lines (xfit, yfit, col="blue")

        xpos <- xylims [1] + 0.5 * diff (xylims)
        ypos <- xylims [1] + c(0.9, 0.8, 0.7) * diff (xylims)
        par (ps=18)
        text (xpos, ypos[1], labels="To ~ From")
        par (ps=12)
        r12 <- cor (x, y, use="pairwise.complete") ^ 2
        p12 <- t.test (x, y, paired=TRUE)$p.value
        text (xpos, ypos[2], labels= paste ("r2 = ",
            formatC (r12, format="f", digits=4), " (T-test p=",
            formatC (p12, format="f", digits=4), ")", sep=""))
    }
}
