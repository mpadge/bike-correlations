# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                     GET.DATA                     *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

get.data <- function (city="nyc", from=TRUE, covar=TRUE, std=TRUE, 
                      nearfar=0, subscriber=0, mf=0,
                      msg=FALSE)
{
    # If !cov, then raw trip numbers are returned
    #
    # subscriber = 0 -> all data
    # subscriber = 2 -> non-subscriber data only
    # subscriber = 1:
    #           mf = 0 -> all data
    #           mf = 1/2 -> male/female data
    # subscriber = 3:
    #           mf = 0/1 -> young/old suscribers

    txt.dir <- "../results/"
    if (subscriber > 2)
        txt.dir <- paste (txt.dir, "age/", sep="")
    fname <- paste (txt.dir, "stationDistsMat-", city, ".csv", sep="")
    dists <- as.matrix (read.csv (fname, header=FALSE))
    dims <- dim (dists)
    dists <- array (dists, dim=dims)

    if (from) txt.ft <- "from"
    else txt.ft <- "to"
    if (nearfar == 0) txt.nf <- "all"
    else if (nearfar == 1) txt.nf <- "near"
    else txt.nf <- "far"
    if (subscriber == 0 | subscriber == 2)
        mf = 0

    if (covar)
    {
        if (std) txt.sd <- "std"
        else txt.sd <- "unstd" # these files don't currently exist
        fname <- paste (txt.dir, "Cov_", city, "_", txt.ft, 
                        "_", txt.sd, "_", txt.nf, sep="")
        indx <- which (dists < 0) # the latter is for -DOUBLE_MAX
        if (city == "nyc" | city == "boston" | city == "chicago")
            fname <- paste (fname, "_", subscriber, mf, sep="")
    } else {
        fname <- paste (txt.dir, "NumTrips_", city, sep="")
        if (city == "nyc" | city == "boston" | city == "chicago")
            fname <- paste (fname, "_", subscriber, mf, sep="")
    }
    fname <- paste (fname, ".csv", sep="")
    y <- as.matrix (read.csv (fname, header=FALSE))
    y <- array (y, dim=dims)

    # covar 
    if (covar)
        indx <- which (dists < 0 | y < (-999999)) # arbitrarily big number
    else
        indx <- which (dists < 0)

    dists [indx] <- NA
    y [indx] <- NA
    # London has some extreme outliers in unstandarsied covariances, so
    if (!std & city == "london")
    {
        indx <- which (y > -min (y, na.rm=TRUE))
        if (msg)
            cat ("London-", txt.ft, ": Removed ", length (indx), " / ", 
                    length (dists), " = ", 
                    formatC (100 * length (indx) / length (dists), 
                    format="f", digits=1), "% extreme covariances.\n", sep="")
        dists [indx] <- NA
        y [indx] <- NA
    }

    return (list (d=dists, y=y))
} # end get.data()


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
    # Produces a data frame with [i, k, y, ss, ntrips, F], where F is an
    # F-statistic for changes in the variance of model residuals with distance.
    # This is calculated by binning them into 20 bins, and so can be tested with
    # p = 1 - pf (mean (f), 1, 18). Note, however, that slopes of these
    # relationships can differ in sign, so perhaps a more robust way to test is
    # simply with a T-test on the distribution of slopes of these residual
    # regressions.
    #
    # Statistics for these residuals are evaluated in the subsequent routine
    # "test.resids"
    nbins <- 20

    dat <- get.data (city=city, from=from, covar=covar, std=std,
                     nearfar=nearfar, subscriber=subscriber,
                     mf=mf, msg=msg)

    rd <- "../results/";
    if (subscriber > 2)
        rd <- paste (rd, "age/", sep="")
    fname <- paste (rd, "NumTrips_", city, sep="")
    if (city == "nyc" | city == "boston" | city == "chicago")
        fname <- paste (fname, "_", subscriber, mf, sep="")
    fname <- paste (fname, ".csv", sep="")
    ntrips.mat <- read.csv (fname, header=FALSE)

    n <- dim (dat$d)[1]

    index <- kvec <- intercept <- ss <- r2 <- ntrips <- 
        resid.slope <- fvals <- NULL

    if (from) ftxt <- "from"
    else ftxt <- "to"
    tstr <- paste (toupper (city), "-", ftxt, sep="")

    for (i in 1:n)
    {
        d <- dat$d [i, ]
        y <- dat$y [i, ]
        indx <- which (!is.na (d) & !is.na (y))
        # There are some cases for which trip data exist yet station distances
        # haven't yet been added to list (because the stations do not appear in
        # the website!) These produce NAs in the dists table.
        # The sd(y) condition prevents analyses of !covar for which all trips,
        # and thus all y-values, are zero.
        if (length (indx) > 10 & sd (y, na.rm=TRUE) > 0)
        {
            d <- d[indx]
            y <- y[indx]
            dfit <- seq(min(d), max(d), length.out=100)
            a0 <- 2 * mean (y)
            k0 <- 1
            mod <- NULL
            while (is.null (mod) & k0 < 5)
            {
                k0 <- k0 + 1
                mod <- tryCatch (nls (y ~ a * exp (-d^2 / k^2), 
                            start = list (a = 2 * mean (y), k = k0)),
                            error = function (e) NULL)
            }
            if (!is.null (mod))
            {
                coeffs <- summary (mod)$coefficients
                if (coeffs [2] > 0 & coeffs [2] < 10)
                {
                    ss.mod <- sum ((predict (mod) - y) ^ 2)
                    ss <- c (ss, ss.mod)
                    # NOTE: These r2 values aren't necessarily meaningful!
                    r2 <- c (r2, 1 - ss.mod / sum ((y - mean (y)) ^ 2))
                    kvec <- c (kvec, coeffs [2])
                    intercept <- c (intercept, coeffs [1])
                    ntrips <- c (ntrips, sum (ntrips.mat [,i]))
                    index <- c (index, i)

                    # Then the F-statistic for variance of residuals
                    resids <- summary (mod)$residuals

                    dindx <- ceiling (1:length (indx) * nbins / length (indx))
                    d2 <- d
                    d2 [order (d)] <- dindx
                    rvar <- sapply (1:nbins, function (ix) 
                                    var (resids [which (dindx == ix)]))
                    xvar <- sapply (1:nbins, function (ix) 
                                    mean (d [which (d2 == ix)]))
                    mod <- summary (lm (rvar ~ xvar))
                    resid.slope <- c (resid.slope, mod$coefficients [2])
                    fvals <- c (fvals, as.numeric (mod$fstatistic [1]))
                }
            }
        } # end if len (indx) > 10
    } # end for i

    # Standardised SS values are means for each model, which have already been
    # standardised to have means of ~O(1/nstations). Actual SS values are
    # calculated above as means, so effectively divided again by nstations, and
    # finally divided again below by nstations below to give final single SS
    # values. With nstations=332 (nyc) or 752 (london), respective values are
    # ultimately divided by 36,594,368 = 3.6e7 and 425,259,008 = 4.2e8.
    if (covar)
        ss <- ss * 1e11
    else
        ss <- ss / 10000

    dat <- data.frame (cbind (index, kvec, intercept, ntrips, ss, r2,
                              resid.slope, fvals)) 
    colnames (dat) <- c("i", "k", "y", "ntrips", "ss", "r2", "resid.slope", "f")
    return (dat)
} # end fit.gaussian()


# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                   TEST.RESIDS                    *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

test.resids <- function (from=TRUE, covar=TRUE)
{
    # Tests the regressions of the variances of model residuals from the above
    # fit.gaussian procedure. A reasonable hypothesis would be that these
    # residuals should have slopes that differ systematically from zero---which
    # is tested with the T-test---and that individual relationships should be
    # significant on average---which is tested with the F-test. Thus only if
    # both of these criteria are fulfilled may it be concluded that there are
    # significant spatial associations in the residuals of the models.

    cities <- c ("london", "nyc", "boston", "chicago", "washingtondc")
    for (ci in cities)
    {
        cat (rep ("-", 20), toupper (ci), rep ("-", 20), "\n", sep="")
        dat <- fit.gaussian (city=ci, from=from, covar=covar)
        cat ("R2 = ", formatC (mean (dat$r2), format="f", digits=2), " +/- ",
             formatC (sd (dat$r2), format="f", digits=2), "\n", sep="")
        fp <- 1 - pf (mean (dat$f), 1, 18)
        tp <- t.test (dat$resid.slope)$p.value
        cat ("Mean F-statistic = ", formatC (mean (dat$f), format="f", digits=2),
             " p (df=1,18) = ", formatC (fp, format="f", digits=4),
             "; T-statistic for slopes of residuals: p = ",
             formatC (tp, format="f", digits=4), "\n", sep="")

        p0 <- 0.05
        if (fp < p0 & tp < p0)
            cat ("Residuals manifest SIGNIFICANT spatial structure!\n")
        else
            cat ("Residuals do not manifest any significant spatial structure\n")
    }
} # end test.resids


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


compare.models <- function (city="nyc", from=TRUE, covar=TRUE, plot=FALSE)
{
    # TODO: Add distributions: Weibull, Cauchy, squared Cauchy (see Halas2014),
    # Box-Cox
    # See Luoma1983 and Halas2014
    #
    # plot allows inspection of individual station results
    if (covar)
        cat ("Fitting distance decays to covariances\n")
    else
        cat ("Fitting distance decays to absolute numbers of trips\n")

    dat <- get.data(city=city, from=from, covar=covar, std=TRUE, 
                    nearfar=0, subscriber=0, mf=0, msg=TRUE)

    n <- dim (dat$d)[1]

    if (covar)
        ssmult <- 1e11
    else
        ssmult <- 1 / 10000
    results <- data.frame (array (NA, dim=c(n, 8)))
    names (results) <- c ("ss.pow", "b.pow", "ss.gauss", "k.gauss", "ss.exp", 
                    "k.exp", "b.exp", "aic.diff")

    for (i in 1:n)
    {
        d <- dat$d [i, ]
        y <- dat$y [i, ]
        indx <- which (!is.na (d) & !is.na (y))
        # There are some cases for which trip data exist yet station distances
        # haven't yet been added to list. These produce NAs in the dists table.
        # The sd(y) condition prevents analyses of !covar for which all trips,
        # and thus all y-values, are zero.
        if (length (indx) > 10 & sd (y, na.rm=TRUE) > 0)
        {
            d <- d[indx]
            y <- y[indx]
            if (plot)
                plot (d, y, pch=1, col="orange")
            dfit <- seq(min(d), max(d), length.out=100)

            # ***** power-law:
            indx <- which (y > 0 & d > 0) # Because 0 distances do occur
            d2 <- d [indx]
            y2 <- y [indx]
            mod <- lm (log10 (y2) ~ log10 (d2))
            coeffs <- summary (mod)$coefficients
            yfit <- 10 ^ (coeffs [1] + log10 (d2) * coeffs [2])
            results$ss.pow [i] <- mean (sum ((yfit - y2) ^ 2)) * ssmult
            results$b.pow [i] <- coeffs [2]
            if (plot)
            {
                yfit <- 10 ^ (coeffs [1] + log10 (dfit) * coeffs [2])
                lines (dfit, yfit, col="red")
                b <- formatC (coeffs [2], format="f", digits=2)
                mtxt <- paste ("[", i, "]: power (b=", b, "): ", 
                               round (results$ss.pow[i]), "\n", sep="")
            }

            # ***** Gaussian:
            a0 <- 2 * mean (y)
            k0 <- 1
            modg <- NULL
            while (is.null (modg) & k0 < 5)
            {
                k0 <- k0 + 1
                modg <- tryCatch (nls (y ~ a * exp (-d^2 / k^2), 
                            start = list (a = 2 * mean (y), k = k0)),
                            error = function (e) NULL)
            }
            if (!is.null (modg))
            {
                results$ss.gauss [i] <- mean (sum ((predict (modg) - y) ^ 2)) *
                               ssmult
                results$k.gauss [i] <- summary (modg)$coefficients [2]
                if (results$k.gauss [i] < 0 | results$k.gauss [i] > 10)
                    results$k.gauss [i] <- results$ss.gauss [i]  <- NA
                if (plot)
                {
                    yfit <- predict (modg, new=data.frame (d=dfit))
                    lines (dfit, yfit, col="blue")
                    mtxt <- paste (mtxt, "Gaussian: ", 
                                   round (results$ss.gauss [i]), "\n", sep="")
                }
            } 
            
            # ***** exponential
            k0 <- 0.1
            modex <- NULL
            while (is.null (modex) & k0 < 5)
            {
                k0 <- k0 + 0.1 
                modex <- tryCatch (nls (y ~ a * exp (-(d / k) ^ b), 
                            start=list(a=2*mean(y),k=k0, b=2)),
                            error=function(e) NULL)
            }
            if (!is.null (modex))
            {
                results$ss.exp [i] <- mean (sum ((predict (modex) - y) ^ 2)) * 
                             ssmult
                results$k.exp [i] <- summary (modex)$coefficients [2]
                results$b.exp [i] <- summary (modex)$coefficients [3]
                if (results$b.exp [i] < 0 | results$b.exp [i] > 9)
                    results$ss.exp [i] <- results$k.exp [i] <- 
                        results$b.exp [i] <- NA
                if (plot)
                {
                    yfit <- predict (modex, new=data.frame (d=dfit))
                    lines (dfit, yfit, col="lawngreen", lwd=2)
                    b <- formatC (results$b.exp [i], format="f", digits=2)
                    mtxt <- paste (mtxt, "Exp (b=", b, "): ", 
                                   round (results$ss.exp [i]), sep="")
                }
            }

            if (!is.null (modg) & !is.null (modex))
                results$aic.diff [i] <- AIC (modg) - AIC (modex)
            # Negative aic.diff indicates Gaussian is preferred

            if (plot)
            {
                title (main = mtxt)
                loc <- locator (n=1)
            }
            else
                cat ("\r", i, "/", n, sep="")
        } # end if len (indx) > 10
    } # end for i
    cat ("\r")

    aic <- mean (results$aic.diff, na.rm=TRUE)
    aicf <- formatC (aic, format="f", digits=2)
    if (aic < 0)
        cat ("Gaussian is preferred over generalised exponential with AIC = ", 
             aicf, "\n", sep="")
    else
        cat ("Generalised exponential is preferred over Gaussian with AIC = ", 
             aicf, "\n", sep="")
    cat ("Model SS errors for [power, Gaussian, exp] = [",
         round (mean (results$ss.pow, na.rm=TRUE)), ", ", 
         round (mean (results$ss.gauss, na.rm=TRUE)), ", ",
         round (mean (results$ss.exp, na.rm=TRUE)), 
         "]; t-test between the latter two: p=",
         formatC (t.test (results$ss.gauss, results$ss.exp)$p.value,
                  format="f", digits=4), "\n", sep="")
    cat ("Estimated exponential and power-law coefficients = (",
         formatC (mean (results$b.exp, na.rm=TRUE), format="f", digits=2), " +/- ",
         formatC (sd (results$b.exp, na.rm=TRUE), format="f", digits=2), ") and (",
         formatC (mean (results$b.pow, na.rm=TRUE), format="f", digits=2), " +/- ",
         formatC (sd (results$b.pow, na.rm=TRUE), format="f", digits=2), 
         ")\n", sep="")
    cat ("Estimated k-values from Gaussian and exponential = (",
         formatC (mean (results$k.gauss, na.rm=TRUE), format="f", digits=2), " +/- ",
         formatC (sd (results$k.gauss, na.rm=TRUE), format="f", digits=2), 
         ") and (",
         formatC (mean (results$k.exp, na.rm=TRUE), format="f", digits=2), " +/- ",
         formatC (sd (results$k.exp, na.rm=TRUE), format="f", digits=2), 
         "); t-test p = ",
         formatC (t.test (results$k.gauss, results$k.exp)$p.value, format="f",
                  digits=4), "\n", sep="")

    return (results)
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
    x11 (width=13.5, height=7)
    par (mfrow=c(3,3), mar=c(2.5,2.5,0.5,0.5), mgp=c(1.3,0.7,0), ps=10)

    cities <- c ("nyc", "washingtondc", "chicago")
    for (city in cities)
    {
        to <- fit.gaussian (city=city, from=FALSE, covar=covar, std=std)
        from <- fit.gaussian (city=city, from=TRUE, covar=covar, std=std)
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

compare.nearfar <- function (from=TRUE, covar=TRUE, std=TRUE)
{
    # nearfar == (1, 2) is (near, far)
    cities <- c ("london", "nyc", "boston", "chicago", "washingtondc")
    for (city in cities)
    {
        dat1 <- fit.gaussian (city=city, from=from, covar=covar, 
                              std=std, nearfar=1)
        dat2 <- fit.gaussian (city=city, from=from, covar=covar, 
                              std=std, nearfar=2)
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

    cities <- cities [2:4]
    for (city in cities)
    {
        # -------------------------------------------------
        # Then comparisons of subscribers and customers (excluding london & DC)
        dat1 <- fit.gaussian (city=city, from=from, covar=covar, std=std, 
                              nearfar=1, subscriber=1, mf=0)
        dat2 <- fit.gaussian (city=city, from=from, covar=covar, std=std, 
                              nearfar=2, subscriber=1, mf=0)
        tt <- t.test (dat1$k, dat2$k)
        cat ("\n", toupper (city), " Subscribers: T = ",
             formatC (tt$statistic, format="f", digits=2), "; df = ",
             formatC (tt$parameter, format="f", digits=1), "; p = ",
             formatC (tt$p.value, format="f", digits=4), "\n", sep="")
        cat ("Mean +/- SD k-values for (near, far) are (",
             formatC (mean (dat1$k), format="f", digits=2), "+/-",
             formatC (sd (dat1$k), format="f", digits=2), ", ",
             formatC (mean (dat2$k), format="f", digits=2), "+/-",
             formatC (sd (dat2$k), format="f", digits=2), ")\n", sep="")
        dat1 <- fit.gaussian (city=city, from=from, covar=covar, std=std, 
                              nearfar=1, subscriber=2, mf=0)
        dat2 <- fit.gaussian (city=city, from=from, covar=covar, std=std, 
                              nearfar=2, subscriber=2, mf=0)
        tt <- t.test (dat1$k, dat2$k)
        cat (toupper (city), " Customers: T = ",
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
        dat1 <- fit.gaussian (city=city, from=from, covar=covar, std=std, 
                              nearfar=1, subscriber=1, mf=2)
        dat2 <- fit.gaussian (city=city, from=from, covar=covar, std=std, 
                              nearfar=2, subscriber=1, mf=2)
        tt <- t.test (dat1$k, dat2$k)
        cat ("\n", toupper (city), " Female: T = ",
             formatC (tt$statistic, format="f", digits=2), "; df = ",
             formatC (tt$parameter, format="f", digits=1), "; p = ",
             formatC (tt$p.value, format="f", digits=4), "\n", sep="")
        cat ("Mean +/- SD k-values for (near, far) are (",
             formatC (mean (dat1$k), format="f", digits=2), "+/-",
             formatC (sd (dat1$k), format="f", digits=2), ", ",
             formatC (mean (dat2$k), format="f", digits=2), "+/-",
             formatC (sd (dat2$k), format="f", digits=2), ")\n", sep="")

        dat1 <- fit.gaussian (city=city, from=from, covar=covar, std=std, 
                              nearfar=1, subscriber=1, mf=1)
        dat2 <- fit.gaussian (city=city, from=from, covar=covar, std=std, 
                              nearfar=2, subscriber=1, mf=1)
        tt <- t.test (dat1$k, dat2$k)
        cat (toupper (city), " Male: T = ",
             formatC (tt$statistic, format="f", digits=2), "; df = ",
             formatC (tt$parameter, format="f", digits=1), "; p = ",
             formatC (tt$p.value, format="f", digits=4), "\n", sep="")
        cat ("Mean +/- SD k-values for (near, far) are (",
             formatC (mean (dat1$k), format="f", digits=2), "+/-",
             formatC (sd (dat1$k), format="f", digits=2), ", ",
             formatC (mean (dat2$k), format="f", digits=2), "+/-",
             formatC (sd (dat2$k), format="f", digits=2), ")\n", sep="")
    }
} # end compare.nearfar()


# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                COMPARE.SUBSCRIBERS               *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

compare.subscribers <- function (city="nyc", covar=TRUE, std=TRUE)
{
    # comparisons of subscribers and customers
    dat1 <- fit.gaussian (city=city, covar=covar, std=std, 
                          nearfar=0, subscriber=1, mf=0)
    dat2 <- fit.gaussian (city=city, covar=covar, std=std, 
                          nearfar=0, subscriber=2, mf=0)
    tt <- t.test (dat1$k, dat2$k)
    cat (toupper (city), " Subscribers/Non-subscribers: T = ",
         formatC (tt$statistic, format="f", digits=2), "; df = ",
         formatC (tt$parameter, format="f", digits=1), "; p = ",
         formatC (tt$p.value, format="f", digits=4), "\n", sep="")
    cat ("Mean +/- SD k-values for (subscribers, non-subscribers) are (",
         formatC (mean (dat1$k), format="f", digits=2), "+/-",
         formatC (sd (dat1$k), format="f", digits=2), ", ",
         formatC (mean (dat2$k), format="f", digits=2), "+/-",
         formatC (sd (dat2$k), format="f", digits=2), ")\n", sep="")

    dat1 <- fit.gaussian (city=city, covar=covar, std=std, 
                          nearfar=0, subscriber=1, mf=1) # male
    dat2 <- fit.gaussian (city=city, covar=covar, std=std, 
                          nearfar=0, subscriber=1, mf=2) # female
    tt <- t.test (dat1$k, dat2$k)
    cat (city, " Male/Female: T = ",
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
    city <- c ("london", rep ("nyc", 8), rep ("boston", 8), rep ("chicago", 8),
               "washingtondc")
    nearfar1 <- c (1, rep (c (1, 1, 1, 1, 1, 0, 0, 0), 3), 1)
    nearfar2 <- c (2, rep (c (2, 2, 2, 2, 2, 0, 0, 0), 3), 2)
    subscriber1 <- c (0, rep (c (0, 1, 2, 1, 1, 1, 1, 3), 3), 0)
    subscriber2 <- c (0, rep (c (0, 1, 2, 1, 1, 2, 1, 3), 3), 0)
    gender1 <- c (0, rep (c (0, 0, 0, 2, 1, 0, 2, 0), 3), 0)
    gender2 <- c (0, rep (c (0, 0, 0, 2, 1, 0, 1, 1), 3), 0)

    nftxt <- c (rep ("NEAR/FAR", 6), rep ("all\t", 3))
    nftxt [10:17] <- nftxt [2:9]
    nftxt [18:25] <- nftxt [2:9]
    nftxt <- c (nftxt, "NEAR/FAR")
    subtxt <- c (rep ("all\t", 2), "subscriber", "customer",
                 rep ("subscriber", 2), "SUB/CUST", rep ("subscriber", 2))
    subtxt [10:17] <- subtxt [2:9]
    subtxt [18:25] <- subtxt [2:9]
    subtxt <- c (subtxt, "all\t")
    gtxt <- c (rep ("all\t", 4), "female\t", "male\t", "all\t", "FEMALE/MALE",
               "all\t")
    gtxt [10:17] <- gtxt [2:9]
    gtxt [18:25] <- gtxt [2:9]
    gtxt <- c (gtxt, "all\t")
    atxt <- c (rep ("all\t", 8), "YOUNG/OLD")
    atxt [10:17] <- atxt [2:9]
    atxt [18:25] <- atxt [2:9]
    atxt <- c (atxt, "all\t")

    cat ("|\tCity\tNear/Far\tSubscriber Status\tGender\t\tAge\t\t",
         "T-value\tp-value\t|\n", sep="")
    cat (rep ("-", 105), "\n", sep="")

    lines <- c (1, 9, 17, 25)
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
        cat (formatC (tt$statistic, format="f", digits=2), "\t", formatC
             (tt$p.value, format="f", digits=4), "\t|\n", sep="")
        if (i %in% lines)
            cat (rep ("-", 105), "\n", sep="")
    }
    cat (rep ("-", 105), "\n", sep="")
}


# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                    SPATIAL.VAR                   *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

spatial.var <- function (city="london")
{
    # Quantifies the extent to which estimates of distance decay functions are
    # same for near vs. far stations.
    tf <- c (FALSE, TRUE)
    tftxt <- c ("TO", "FROM")
    fht <- 3.5
    mfrow <- c (1, 2)
    if (city != "london" & city != "washingtondc")
    {
        fht <- 8
        mfrow <- c (3, 2)
    }
    x11 (width=7, height=fht)
    par (mfrow=mfrow, mar=c(2.5,2.5,2.5,0.5), mgp=c(1.3,0.7,0), ps=12)

    for (i in 1:2)
    {
        dat1 <- fit.gaussian (city=city, nearfar=1, from=tf [i])
        dat2 <- fit.gaussian (city=city, nearfar=2, from=tf [i])
        n <- max (c (dat1$i, dat2$i))
        k1 <- k2 <- rep (NA, n)
        k1 [dat1$i] <- dat1$k
        k2 [dat2$i] <- dat2$k
        nin <- length (which (!is.na (k1) & !is.na (k2)))
        plot (k1, k2, pch=1, col="lawngreen", xlab="near", ylab="far")
        mod <- lm (k2 ~ k1)
        x <- seq (min (k1,na.rm=TRUE), max(k1,na.rm=TRUE), length.out=100)
        y <- predict (mod, new=data.frame (k1=x))
        lines (x, y, col="blue", lwd=2)
        r2 <- formatC (summary (mod)$r.squared, format="f", digits=2)
        title (main=paste (tftxt [i], ": R2 = ", r2, " (n=", nin,
                           "/", n, ")", sep=""))
    }
    xpos <- min (k1, na.rm=TRUE) + 0.8 * diff (range (k1, na.rm=TRUE))
    ypos <- min (k2, na.rm=TRUE) + 0.8 * diff (range (k2, na.rm=TRUE))
    par (ps=18)
    text (xpos, ypos, labels=toupper (city))
    par (ps=12)

    # subscriber = 0 -> all data
    # subscriber = 2 -> non-subscriber data only
    # subscriber = 1:
    #           mf = 0 -> all data
    #           mf = 1/2 -> male/female data
    # subscriber = 3:
    #           mf = 0/1 -> young/old suscribers
    if (city != "london" & city != "washingtondc")
    {
        subs <- c (1, 3)
        mf1 <- c (1, 0)
        mf2 <- c (2, 1)
        xtxt <- c ("male", "young")
        ytxt <- c ("female", "old")
        for (i in 1:2) # over (gender, age)
            for (j in 1:2) # over (to, from)
            {
                dat1 <- fit.gaussian (city=city, nearfar=0, from=tf [j],
                                      subscriber=subs [i], mf=mf1 [i])
                dat2 <- fit.gaussian (city=city, nearfar=0, from=tf [j],
                                      subscriber=subs [i], mf=mf2 [i])
                n <- max (c (dat1$i, dat2$i))
                k1 <- k2 <- rep (NA, n)
                k1 [dat1$i] <- dat1$k
                k2 [dat2$i] <- dat2$k
                nin <- length (which (!is.na (k1) & !is.na (k2)))
                plot (k1, k2, pch=1, col="lawngreen", 
                      xlab=xtxt [i], ylab=ytxt [i])
                mod <- lm (k2 ~ k1)
                x <- seq (min (k1,na.rm=TRUE), max(k1,na.rm=TRUE), length.out=100)
                y <- predict (mod, new=data.frame (k1=x))
                lines (x, y, col="blue", lwd=2)
                r2 <- formatC (summary (mod)$r.squared, format="f", digits=2)
                p <- formatC (summary (mod)$coefficients [8], format="f", digits=4)
                title (main=paste (tftxt [i], ": R2 = ", r2, " (n=", nin,
                                   "/", n, "; p=", p, ")", sep=""))
            } # end for j
    }
} # end spatial.var()


# ************************************************************ 
# ************************************************************ 
# *****                                                  *****
# *****                    PLOT.HISTS                    *****
# *****                                                  *****
# ************************************************************ 
# ************************************************************ 

plot.hists <- function (covar=TRUE)
{
    require (segmented)

    # if !covar, then hists of trip fluxes are plotted
    city <- c ("nyc", "boston", "chicago", "washingtondc", "london")
    ntabs <- c (1, 1, 1, 0, 1)
    cov.mult <- 1e5

    x11 (width=10, height=6)
    layout (matrix (c (1, 1, 1, 2:7), 3, 3, byrow=TRUE), 
            height=c (0.1, 0.45, 0.45))
    cols <- c ("red", "blue", "grey") # (from, to, regression line)

    par (mar=rep (0, 4), mgp=rep (0, 3))
    plot (NULL, xlim=c(0, 1), ylim=c(0, 1), 
          xlab="", ylab="", xaxt="n", yaxt="n", frame=FALSE)
    par (ps=24)
    if (covar)
        mt <- "Covariance"
    else
        mt <- "Numbers of trips"
    text (0.5, 0.4, labels=mt, pos=3)
    if (covar)
        mt <- expression (paste (Covariance %*% 10^5, sep=""))
    par (mar=c(3,3,2.5,0.5), mgp=c(1.8,0.7,0), ps=12)

    yall <- NULL
    if (covar)
    {
        ft <- c (TRUE, FALSE)
        ft.txt <- c ("from", "to")
        cat ("\t|\tCITY\t\tDIR\t|\tslope1\tslope2\tbreak point\t|\n")
        cat ("\t|", rep("-", 71), "|\n", sep="")
    }
    else
    {
        cat ("\t|\tCITY\t\t|\tslope\t|\n", sep="")
        cat ("\t|", rep("-", 39), "|\n", sep="")
    }

    for (ci in 1:length (city))
    {
        if (covar)
        {
            dat <- x <- y <- list ()
            for (i in 1:2)
            {
                dat [[i]] <- get.data (city=city [ci], from=ft [i], covar=TRUE)
                dat [[i]]$y <- dat [[i]]$y * cov.mult
                yall <- c (yall, dat [[i]]$y)
                hh <- hist (dat[[i]]$y, breaks=101, plot=FALSE)
                # Plots are only taken up to first count of zero - rest is noise
                indx <- min (which (hh$mids > 0)):(which (hh$counts == 0)[1] - 1)
                x [[i]] <- hh$mids [indx]
                y [[i]] <- hh$counts [indx]
            }
        }
        else
        {
            dat <- get.data (city=city [ci], covar=FALSE)
            yall <- c (yall, dat$y)
            hh <- hist (dat$y, breaks=101, plot=FALSE)
            # Plots are only taken up to first count of zero - rest is noise
            indx <- min (which (hh$mids > 0)):(which (hh$counts == 0)[1] - 1)
            x <- hh$mids [indx]
            y <- hh$counts [indx]
        }

        ylims <- range (y, na.rm=TRUE)

        if (covar)
        {
            plot (x [[1]], y [[1]], "l", log="xy", lwd=2, col=cols [1], 
                  ylim=ylims, xlab=mt, ylab="Frequency", main=city [ci])
            lines (x [[2]], y [[2]], col=cols [2], lwd=2)
            xlims <- range (x, na.rm=TRUE)
            xpos <- xlims [1] + 0.2 * diff (xlims)
            legend (xpos, ylims [2], lwd=2, col=cols, bty="n", legend=ft.txt)

            for (i in 1:2)
            {
                xl <- log10 (x [[i]])
                yl <- log10 (y [[i]])
                cat ("\t|\t", city [ci], "\t", rep ("\t", ntabs [ci]), 
                                             ft.txt [i], "\t|\t", sep="")
                # Then fit the segmented linear model
                mod <- segmented (lm (yl ~ xl), seg.Z = ~xl, psi=mean (xl)) 
                slope1 <- formatC (slope (mod)$xl [1], format="f", digits=2)
                slope2 <- formatC (slope (mod)$xl [2], format="f", digits=2)
                bp <- 10 ^ mod$psi [2]
                breakpt <- formatC (bp, format="f", digits=1)
                cat (slope1, "\t", slope2, "\t", breakpt, "\t\t|\n")
                yfit <- 10 ^ predict (mod)
                lines (x [[i]], yfit, col=cols [i], lwd=1, lty=2)
                bi <- which.min (abs (x [[i]] - bp))
                points (x [[i]] [bi], yfit [bi], pch=19, col=cols [i], cex=1.5)
                lines (rep (bp, 2), c (ylims [1], yfit [bi]), 
                       col=cols [i], lty=2)
            }
        }
        else # !covar
        {
            plot (x, y, "l", log="xy", lwd=2, col=cols [1], 
                  ylim=ylims, xlab=mt, ylab="Frequency", main=city [ci])
            xl <- log10 (x)
            yl <- log10 (y)
            mod <- lm (yl ~ xl)
            slope <- formatC (summary (mod)$coefficients [2],
                              format="f", digits=2)
            cat ("\t|\t", city [ci], rep ("\t", ntabs [ci]), 
                 "\t|\t", slope, "\t|\n", sep="")
            yfit <- 10 ^ predict (mod)
            lines (x, yfit, col=cols [3], lwd=2)
        }
    } # end for ci over city

    hh <- hist (yall, breaks=101, plot=FALSE)
    indx <- min (which (hh$mids > 0)):(which (hh$counts == 0)[1] - 1)
    x <- hh$mids [indx]
    y <- hh$counts [indx]

    ylims <- range (y, na.rm=TRUE)
    plot (x, y, "l", log="xy", lwd=2, col=cols [1], ylim=ylims,
          xlab=mt, ylab="Frequency", main="All")

    yl <- log10 (y)
    xl <- log10 (x)
    mod <- lm (yl ~ xl)
    slope <- formatC (summary (mod)$coefficients [2], format="f", digits=2)
    yfit <- 10 ^ predict (mod)
    lines (x, yfit, col=cols [3], lwd=2)
    if (covar)
    {
        cat ("\t|", rep("-", 71), "|\n", sep="")
        cat ("\t|\tALL\t\t\t|\t", slope, "\t\t\t\t|\n")
        cat ("\t|", rep("-", 71), "|\n", sep="")
    }
    else
    {
        cat ("\t|", rep("-", 39), "|\n", sep="")
        cat ("\t|\tALL\t\t|\t", slope, "\t|\n", sep="")
        cat ("\t|", rep("-", 39), "|\n", sep="")
    }
}
