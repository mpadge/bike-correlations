# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                     GET.DATA                     *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

get.data <- function (from=TRUE, covar=TRUE, tube=TRUE, nearfar=0, msg=FALSE)
{
    txt.dir <- "../results/trains/"
    if (tube)
        fname <- paste (txt.dir, "DistMatTube.csv", sep="")
    else
        fname <- paste (txt.dir, "DistMatRail.csv", sep="")
    dists <- as.matrix (read.csv (fname, header=FALSE))
    dims <- dim (dists)
    dists <- array (dists, dim=dims)

    if (from) txt.ft <- "from"
    else txt.ft <- "to"
    if (tube) txt.tube <- "tube"
    else txt.tube <- "rail"

    if (covar)
        fname <- paste (txt.dir, "Cov_london_", txt.tube, "_", txt.ft, 
                        "_all.csv", sep="")
    else
        fname <- paste (txt.dir, "R2_london_", txt.tube, "_", txt.ft, 
                        "_all.csv", sep="")

    y <- as.matrix (read.csv (fname, header=FALSE))
    y <- array (y, dim=dims)

    return (list (d=dists, y=y))
} # end get.data()

# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                     FIT.DECAY                    *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

fit.decay <- function (tube=TRUE, from=TRUE, mod.type="power", covar=TRUE, 
                       ylims=NULL, plot=TRUE)
{
    msg <- FALSE
    if (plot) msg <- TRUE
    # If !covar, then models are fitted to R2 values, otherwise they are fitted
    # to covariances.
    dat <- get.data (from=from, covar=covar, tube=tube, nearfar=0, msg=msg)

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
                tstr <- "power law"
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
                tstr <- "Gaussian"
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
                tstr <- "log-Gaussian"
            }
        } # end if len (indx) > 10
    } # end for i
    # Any cases that generate k-values that are greater than max (d) are very
    # likely spurious, so are removed here
    maxd <- max (dat$d, na.rm=TRUE)
    indx <- which (kvec > maxd)
    if (msg & length (indx) > 0)
        cat (mod.type, "-", ftxt, ": Removed ", length (indx), 
             " k-values > d = ", maxd, "km\n", sep="")
    indx <- kvec < maxd
    ss <- ss [indx]
    index <- index [indx]
    kvec <- kvec [indx]
    intercept <- intercept [indx] 

    # Standardised SS values are means for each model, which have already been
    # standardised to have means of ~O(1/nstations). Actual SS values are
    # calculated above as means, so effectively divided again by nstations, and
    # finally divided again below by nstations below to give final single SS
    # values. With nstations=332 (nyc) or 752 (london), respective values are
    # ultimately divided by 36,594,368 = 3.6e7 and 425,259,008 = 4.2e8.
    if (covar)
        ss <- ss * 1e10
    else
        ss <- ss * 100

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
} # end fit.decay()


# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                   FIT.GAUSSIAN                   *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

fit.gaussian <- function (city="nyc", from=TRUE, covar=TRUE, std=TRUE,
                          nearfar=0, subscriber=0, mf=0, msg=FALSE)
{
    # Produces a data frame with [i, k, y, ss, ntrips]
    # If !covar, then models are fitted to R2 values, otherwise they are fitted
    # to covariances.
    dat <- get.data (city=city, from=from, covar=covar, std=std,
                     nearfar=nearfar, subscriber=subscriber,
                     mf=mf, msg=msg)
    rd <- "../results/";
    if (subscriber > 2)
        rd <- paste (rd, "age/", sep="")
    fname <- paste (rd, "/NumTrips_", city, sep="")
    if (city == "nyc")
        fname <- paste (fname, "_", subscriber, mf, sep="")
    fname <- paste (fname, ".csv", sep="")
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
        # haven't yet been added to list (because the stations do not appear in
        # the website!) These produce NAs in the dists table.
        if (length (indx) > 2)
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
    # likely spurious, so are removed here (but only for nearfar == 0)
    if (nearfar == 0)
    {
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
    }

    # See note in fit.decay for these values
    if (std)
        ss <- ss * 1e12
    else if (covar)
        ss <- ss / 1e7
    dat <- data.frame (cbind (index, kvec, intercept, ntrips, ss)) 
    colnames (dat) <- c("i", "k", "y", "ntrips", "ss")
    return (dat)
} # end fit.gaussian()

# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                  COMPARE.NTRIPS                  *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

compare.ntrips <- function (covar=TRUE, std=TRUE)
{
    # These results with std=TRUE are statistical artefacts, because
    # correlations involving stations with high numbers of trips will
    # correspondingly adjust all others to extremely low values, giving
    # correspondingly low covariances and *average* squared errors.
    #
    # There will thus arise a negative relationship between numbers of trips at
    # each station and the covariance intercept of the fitted decay model.
    #
    # Numbers of trips may nevertheless theoretically be related to
    # correlations, even though a presumption that correlations should follow
    # defined distance decay functions is highly questionable. Nevertheless,
    # setting covar=FALSE demonstrates that this is not in fact the case.
    # Although London does yield significant slopes for R2~ntrips, their
    # magnitudes reflect the cluster of low-ntrips--low-R2 values, which are
    # obviously statistical noise. A proportion of these values can be removed
    # with the code below, to yield slopes that are far less significant.
    #
    # Relationships between numbers of trips and either covariances or
    # correlations are thus inevitably confounded and may not be given any
    # objective consideration. This function thus serves only to produce the
    # other half of the resultant relationships, which are relationships between
    # numbers of trips and k-values, which are also not significant. This is the
    # major and interesting finding here.
    cities <- c ("london", "nyc")
    x11 (width=14)
    par (mfrow=c(2,4), mar=c(2.5,2.5,0.5,0.5), mgp=c(1.3,0.7,0), ps=10)
    ft <- c (TRUE, FALSE)
    ftxt <- c ("from", "to")
    if (covar) ytxt <- c ("k-value", "Covariance")
    else ytxt <- c ("k-value", "R2")
    for (city in cities)
    {
        for (i in 1:2) {
            dat <- fit.gaussian (city=city, from=ft[i], covar=covar, std=std,
                                 nearfar=0, subscriber=0, mf=0)
            n <- dat$ntrips
            yvals <- list (k=dat$k, covar=dat$y)
            if (!covar)
            {
                # Remove stations with the lowest 10% of ntrips
                indx <- sort (n, index.return=TRUE)$ix
                indx <- indx [1:floor (0.2 * length (indx))]
                #yvals$covar [indx] <- NA
            }
            for (j in 1:2) {
                yj <- yvals [[j]]
                plot (n, yj, pch=1, col="orange", bty="l",
                      xlab="ntrips", ylab=ytxt [j])
                mod <- lm (yj ~ n)
                x <- seq (min (n), max (n), length.out=100)
                y <- predict (mod, new=data.frame(n=x))
                lines (x, y, col="blue")

                r2 <- summary (mod)$r.squared
                pval <- summary (mod)$coefficients [8]
                xpos <- min (n) + 0.5 * diff (range (n))
                ylims <- range (yj, na.rm=TRUE)
                ypos <- ylims[1] + c(0.8, 0.6, 0.4) * diff (ylims)
                par (ps=18)
                text (xpos, ypos[1], labels=paste (toupper (city), ftxt [i]))
                par (ps=12)
                text (xpos, ypos[2], labels= paste ("r2 = ",
                    formatC (r2, format="f", digits=4), " (p=",
                    formatC (pval, format="f", digits=4), ")", sep=""))
                par (ps=10)
            } # end for j
        } # end for i
    } # end for city
} # end compare.ntrips()

# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                  COMPARE.MODELS                  *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

compare.models <- function (city="nyc", from=TRUE, covar=TRUE, std=TRUE)
{
    mod.types <- c ("power", "Gaussian", "logGauss")
    x11 ()
    par (mfrow=c(2,2), mar=c(2.5,2.5,0.5,0.5), mgp=c(1.3,0.7,0), ps=10)
    fulldat <- list()
    for (i in 1:3)
        fulldat [[i]] <- fit.decay(city=city, from=from, mod.type=mod.types[i],
                                   covar=covar, std=std, plot=TRUE)
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
} # end compare.models()


# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                  COMPARE.TOFROM                  *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

compare.tofrom <- function (covar=TRUE, std=TRUE, paired=FALSE)
{
    # "paired" is fed to the t.test for differences in estimated k-values
    # between to- and from- values. If the two data sets are truly presumed to
    # be independent, then there ought also be *no* corresponding dependence
    # between values measured at individual stations, and thus paired should
    # arguably be FALSE.
    x11 (width=13.5)
    par (mfrow=c(2,3), mar=c(2.5,2.5,0.5,0.5), mgp=c(1.3,0.7,0), ps=10)

    cities <- c ("nyc", "london")
    for (city in cities)
    {
        to <- fit.decay(city=city, from=FALSE, mod.type="Gaussian", covar=covar, 
                            std=std, ylims=c(0, 10))
        from <- fit.decay(city=city, from=TRUE, mod.type="Gaussian", covar=covar, 
                            std=std, ylims=c(0, 10))
        # matrices of correlations and t-tests between k-values
        imax <- max (c (max (to$i), max (from$i)))
        x <- y <- rep (NA, imax)
        x [to$i] <- to$k
        y [from$i] <- from$k

        xylims <- c (0, min (c (max (x, na.rm=TRUE), max (y, na.rm=TRUE))))
        plot (x, y, pch=1, col="lawngreen", bty="l", xlim=xylims, ylim=xylims,
              xlab="TO k-values", ylab="FROM k-values")
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
        p12 <- t.test (x, y, paired=paired)$p.value
        text (xpos, ypos[2], labels= paste ("r2 = ",
            formatC (r12, format="f", digits=4), " (T-test p=",
            formatC (p12, format="f", digits=4), ")", sep=""))
    }
} # end compare.tofrom()


# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                  COMPARE.NEARFAR                 *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

compare.nearfar <- function (covar=TRUE, std=TRUE)
{
    # nearfar == (1, 2) is (near, far)
    cities <- c ("london", "nyc")
    for (city in cities)
    {
        dat1 <- fit.gaussian (city=city, covar=covar, std=std, nearfar=1)
        dat2 <- fit.gaussian (city=city, covar=covar, std=std, nearfar=2)
        tt <- t.test (dat1$k, dat2$k)
        cat (toupper (city), ": T = ",
             formatC (tt$statistic, format="f", digits=2), "; df = ",
             formatC (tt$parameter, format="f", digits=1), "; p = ",
             formatC (tt$p.value, format="f", digits=4), "\n", sep="")
        cat ("Mean +/- SD k-values for (near, far) are (",
             formatC (mean (dat1$k), format="f", digits=2), "+/-",
             formatC (sd (dat1$k), format="f", digits=2), ", ",
             formatC (mean (dat2$k), format="f", digits=2), "+/-",
             formatC (sd (dat2$k), format="f", digits=2), ")\n", sep="")
    }

    # -------------------------------------------------
    # Then NYC comparisons of subscribers and customers
    dat1 <- fit.gaussian (city="nyc", covar=covar, std=std, 
                          nearfar=1, subscriber=1, mf=0)
    dat2 <- fit.gaussian (city="nyc", covar=covar, std=std, 
                          nearfar=2, subscriber=1, mf=0)
    tt <- t.test (dat1$k, dat2$k)
    cat ("\nNYC Subscribers: T = ",
         formatC (tt$statistic, format="f", digits=2), "; df = ",
         formatC (tt$parameter, format="f", digits=1), "; p = ",
         formatC (tt$p.value, format="f", digits=4), "\n", sep="")
    cat ("Mean +/- SD k-values for (near, far) are (",
         formatC (mean (dat1$k), format="f", digits=2), "+/-",
         formatC (sd (dat1$k), format="f", digits=2), ", ",
         formatC (mean (dat2$k), format="f", digits=2), "+/-",
         formatC (sd (dat2$k), format="f", digits=2), ")\n", sep="")
    dat1 <- fit.gaussian (city="nyc", covar=covar, std=std, 
                          nearfar=1, subscriber=2, mf=0)
    dat2 <- fit.gaussian (city="nyc", covar=covar, std=std, 
                          nearfar=2, subscriber=2, mf=0)
    tt <- t.test (dat1$k, dat2$k)
    cat ("NYC Customers: T = ",
         formatC (tt$statistic, format="f", digits=2), "; df = ",
         formatC (tt$parameter, format="f", digits=1), "; p = ",
         formatC (tt$p.value, format="f", digits=4), "\n", sep="")
    cat ("Mean +/- SD k-values for (near, far) are (",
         formatC (mean (dat1$k), format="f", digits=2), "+/-",
         formatC (sd (dat1$k), format="f", digits=2), ", ",
         formatC (mean (dat2$k), format="f", digits=2), "+/-",
         formatC (sd (dat2$k), format="f", digits=2), ")\n", sep="")

    # -------------------------------------------------
    # Then male and female, which can only be done for subscribers
    dat1 <- fit.gaussian (city="nyc", covar=covar, std=std, 
                          nearfar=1, subscriber=1, mf=2)
    dat2 <- fit.gaussian (city="nyc", covar=covar, std=std, 
                          nearfar=2, subscriber=1, mf=2)
    tt <- t.test (dat1$k, dat2$k)
    cat ("\nNYC Female: T = ",
         formatC (tt$statistic, format="f", digits=2), "; df = ",
         formatC (tt$parameter, format="f", digits=1), "; p = ",
         formatC (tt$p.value, format="f", digits=4), "\n", sep="")
    cat ("Mean +/- SD k-values for (near, far) are (",
         formatC (mean (dat1$k), format="f", digits=2), "+/-",
         formatC (sd (dat1$k), format="f", digits=2), ", ",
         formatC (mean (dat2$k), format="f", digits=2), "+/-",
         formatC (sd (dat2$k), format="f", digits=2), ")\n", sep="")

    dat1 <- fit.gaussian (city="nyc", covar=covar, std=std, 
                          nearfar=1, subscriber=1, mf=1)
    dat2 <- fit.gaussian (city="nyc", covar=covar, std=std, 
                          nearfar=2, subscriber=1, mf=1)
    tt <- t.test (dat1$k, dat2$k)
    cat ("NYC Male: T = ",
         formatC (tt$statistic, format="f", digits=2), "; df = ",
         formatC (tt$parameter, format="f", digits=1), "; p = ",
         formatC (tt$p.value, format="f", digits=4), "\n", sep="")
    cat ("Mean +/- SD k-values for (near, far) are (",
         formatC (mean (dat1$k), format="f", digits=2), "+/-",
         formatC (sd (dat1$k), format="f", digits=2), ", ",
         formatC (mean (dat2$k), format="f", digits=2), "+/-",
         formatC (sd (dat2$k), format="f", digits=2), ")\n", sep="")
} # end compare.nearfar()


# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                COMPARE.SUBSCRIBERS               *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

compare.subscribers <- function (covar=TRUE, std=TRUE)
{
    # NYC comparisons of subscribers and customers
    dat1 <- fit.gaussian (city="nyc", covar=covar, std=std, 
                          nearfar=0, subscriber=1, mf=0)
    dat2 <- fit.gaussian (city="nyc", covar=covar, std=std, 
                          nearfar=0, subscriber=2, mf=0)
    tt <- t.test (dat1$k, dat2$k)
    cat ("NYC Subscribers/Non-subscribers: T = ",
         formatC (tt$statistic, format="f", digits=2), "; df = ",
         formatC (tt$parameter, format="f", digits=1), "; p = ",
         formatC (tt$p.value, format="f", digits=4), "\n", sep="")
    cat ("Mean +/- SD k-values for (subscribers, non-subscribers) are (",
         formatC (mean (dat1$k), format="f", digits=2), "+/-",
         formatC (sd (dat1$k), format="f", digits=2), ", ",
         formatC (mean (dat2$k), format="f", digits=2), "+/-",
         formatC (sd (dat2$k), format="f", digits=2), ")\n", sep="")

    dat1 <- fit.gaussian (city="nyc", covar=covar, std=std, 
                          nearfar=0, subscriber=1, mf=1) # male
    dat2 <- fit.gaussian (city="nyc", covar=covar, std=std, 
                          nearfar=0, subscriber=1, mf=2) # female
    tt <- t.test (dat1$k, dat2$k)
    cat ("NYC Male/Female: T = ",
         formatC (tt$statistic, format="f", digits=2), "; df = ",
         formatC (tt$parameter, format="f", digits=1), "; p = ",
         formatC (tt$p.value, format="f", digits=4), "\n", sep="")
    cat ("Mean +/- SD k-values for (male, female) are (",
         formatC (mean (dat1$k), format="f", digits=2), "+/-",
         formatC (sd (dat1$k), format="f", digits=2), ", ",
         formatC (mean (dat2$k), format="f", digits=2), "+/-",
         formatC (sd (dat2$k), format="f", digits=2), ")\n", sep="")
} # end compare.subscribers()


# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                    COMPARE.AGE                   *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

compare.age <- function (covar=TRUE, std=TRUE)
{
    years <- (192:199) * 10
    kmn <- ksd <- rep (NA, length (years)) # Just used to get ylims
    dat <- list ()
    for (i in 1:length (years))
    {
        dat [[i]] <- fit.gaussian (city="nyc", covar=covar, std=std,
                                   subscriber=3, mf=years [i])
        kmn [i] <- mean (dat [[i]]$k)
        ksd [i] <- sd (dat [[i]]$k)
        cat (".", sep="")
    }
    cat ("\n")
    age <- 2015 - years

    xlims <- range (age)
    ylims <- range (c (kmn - ksd, kmn + ksd))

    # Convert to single vectors for boxplot
    kvals <- unlist (sapply (dat, function (x) x$k))
    ages <- NULL
    for (i in 1:length (years))
        ages <- c (ages, rep (age [i], dim (dat [[i]]) [1]))
    boxplot (kvals ~ ages, notch=TRUE, ylim=ylims, col="lawngreen")

    maxage <- 90
    indx <- which (ages < maxage)
    ages <- ages [indx]
    kvals <- kvals [indx]
    mod <- lm (kvals ~ ages)
    age <- age [which (age < maxage)]
    fit <- predict (mod, new=data.frame (ages=age))

    par (new=TRUE)
    plot (age, fit, "l", col="blue", lwd=2, xlim=xlims, ylim=ylims,
          xaxt="n", yaxt="n", xlab="", ylab="", frame=FALSE)
    r2 <- formatC (summary (mod)$r.squared, format="f", digits=2)
    p <- formatC (summary (mod)$coefficients [8], format="f", digits=4)
    title (main=paste ("R2 = ", r2, " (p = ", p, ")", sep=""))
    
    dat1 <- fit.gaussian (city="nyc", covar=covar, std=std, 
                          nearfar=0, subscriber=3, mf=0) # young
    dat2 <- fit.gaussian (city="nyc", covar=covar, std=std, 
                          nearfar=0, subscriber=3, mf=1) # old
    tt <- t.test (dat1$k, dat2$k)
    cat ("NYC Young/Old: T = ",
         formatC (tt$statistic, format="f", digits=2), "; df = ",
         formatC (tt$parameter, format="f", digits=1), "; p = ",
         formatC (tt$p.value, format="f", digits=4), "\n", sep="")
    cat ("Mean +/- SD k-values for (young, old) are (",
         formatC (mean (dat1$k), format="f", digits=2), "+/-",
         formatC (sd (dat1$k), format="f", digits=2), ", ",
         formatC (mean (dat2$k), format="f", digits=2), "+/-",
         formatC (sd (dat2$k), format="f", digits=2), ")\n", sep="")
} # end compare.age()


# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                   SUMMARY.STATS                  *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

summary.stats <- function (covar=TRUE, std=TRUE)
{
    cat ("-----T-statistics for pairwise comparisons between k-values-----\n")
    cat ("NOTE: Comparisons are ordered as written, so negative T-values",
         "mean the first value is lower\n\n")
    # Direct comparison of all T-statistics 
    city <- c ("london", rep ("nyc", 8))
    nearfar1 <- c (1, 1, 1, 1, 1, 1, 0, 0, 0)
    nearfar2 <- c (2, 2, 2, 2, 2, 2, 0, 0, 0)
    subscriber1 <- c (0, 0, 1, 2, 1, 1, 1, 1, 3)
    subscriber2 <- c (0, 0, 1, 2, 1, 1, 2, 1, 3)
    gender1 <- c (0, 0, 0, 0, 2, 1, 0, 2, 0)
    gender2 <- c (0, 0, 0, 0, 2, 1, 0, 1, 1)

    nftxt <- c (rep ("NEAR/FAR", 6), rep ("all\t", 3))
    subtxt <- c (rep ("all\t", 2), "subscriber", "customer",
                 rep ("subscriber", 2), "SUB/CUST", rep ("subscriber", 2))
    gtxt <- c (rep ("all\t", 4), "female\t", "male\t", "all\t", "FEMALE/MALE",
               "all\t")
    atxt <- c (rep ("all\t", 8), "YOUNG/OLD")

    cat ("|\tCity\tNear/Far\tSubscriber Status\tGender\t\tAge\t\t",
         "T-value\tp-value\t|\n", sep="")
    cat (rep ("-", 105), "\n", sep="")

    for (i in 1:length (city))
    {
        cat ("|\t", city [i], "\t", nftxt [i], "\t", subtxt [i], "\t\t", 
             gtxt [i], "\t", atxt [i], "\t", sep="")

        dat1 <- fit.gaussian (city=city[i], covar=covar, std=std, 
                              nearfar=nearfar1[i], 
                              subscriber=subscriber1[i], mf=gender1[i])
        dat2 <- fit.gaussian (city=city[i], covar=covar, std=std, 
                              nearfar=nearfar2[i], 
                              subscriber=subscriber2[i], mf=gender2[i]) 
        tt <- t.test (dat1$k, dat2$k)
        cat (formatC (tt$statistic, format="f", digits=2), "\t",
         formatC (tt$p.value, format="f", digits=4), "\t|\n", sep="")
    }
    cat (rep ("-", 105), "\n", sep="")
}
