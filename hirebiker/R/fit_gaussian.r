#' fit_gaussian
#'
#' Produces a data frame with [i, k, y, ss, ntrips, F], where F is an
#' F-statistic for changes in the variance of model residuals with distance.
#' This is calculated by binning them into 20 bins, and so can be tested with
#' p = 1 - pf (mean (f), 1, 18). Note, however, that slopes of these
#' relationships can differ in sign, so perhaps a more robust way to test is
#' simply with a T-test on the distribution of slopes of these residual
#' regressions.
#'
#' Curves may not be able to be fitted to every station, and results include
#' only those stations for which fits were possible. Statistics for residuals
#' can be evaluated with "test.resids".
#'
#' @param city nyc, washington, chicago, boston, london (case insensitive)
#' @param from TRUE or FALSE
#' @param measure can be ('covar', 'ntrips', 'info'), where the latter is mutual
#' information.
#' @param std is not implemented
#' @param nearfar = (0,1,2) for (all, near, far)
#' @param subscriber = 0 to analyse all data; 1 to analyse subscribers only; 2
#' to analyse non-subscribers only; 3 to perform aged-based analyses
#' @param mf = 1/2 for male/female data (for subscriber=1 only)
#' @param data.dir = directory containing results from the C++ routines
#' @return a data frame of 8 columns (i,k,y,ntrips,ss,r2,resid.slope,f), where i
#' is station number, k is variance of Gaussian fit, resid.slope is measured
#' against distance, and f is the corresponding f-statistic

fit_gaussian <- function (city="nyc", from=TRUE, measure='covar', std=TRUE,
                          nearfar=0, subscriber=0, mf=0, msg=FALSE,
                          data.dir="../results/")
{
    nbins <- 20

    dat <- get_bike_data (city=city, from=from, measure=measure, std=std,
                     nearfar=nearfar, subscriber=subscriber,
                     mf=mf, data.dir=data.dir, msg=msg)

    if (subscriber > 2)
        data.dir <- paste (data.dir, "age/", sep="")
    fname <- paste (data.dir, "NumTrips_", city, sep="")
    if (city == "nyc" | city == "boston" | city == "chicago")
        fname <- paste (fname, "_", subscriber, mf, sep="")
    fname <- paste (fname, ".csv", sep="")
    ntrips.mat <- read.csv (fname, header=FALSE)

    n <- dim (dat$d)[1]

    index <- kvec <- intercept <- ss <- r2 <- ntrips <- 
        resid.slope <- fvals <- NULL

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
            k0 <- 1
            mod <- NULL
            while (is.null (mod) & k0 < 10)
            {
                k0 <- k0 + 1
                # See note in compare_models for explanation of the y0 term.
                mod <- tryCatch (nls (y ~ y0 + a * exp (-d^2 / k^2), 
                            start = list (a=diff (range(y)), k=k0, y0=2*mean(y))),
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
    if (measure == 'covar')
        ss <- ss * 1e12
    else
        ss <- ss / 10000

    results <- data.frame (cbind (dat$xy$lon [index], dat$xy$lat [index], 
                                  index, kvec, intercept, ntrips, ss, r2,
                                  resid.slope, fvals)) 
    colnames (results) <- c("x", "y", "i", "k", "y0", "ntrips", "ss", "r2", 
                            "resid.slope", "f")
    return (results)
} 
