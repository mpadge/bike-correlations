#' plot_vgram
#'
#' Plots spatial semivariogram of k-values.
#'
#' @param city nyc, washington, chicago, boston, london (case insensitive)
#' @param from TRUE or FALSE
#' @param measure can be ('covar', 'ntrips', 'info'), where the latter is mutual
#' information.
#' @param osm=FALSE uses straight line distances, otherwise OSM routed ones
#' @param std is not implemented
#' @param nearfar = (0,1,2) for (all, near, far)
#' @param subscriber = 0 to analyse all data; 1 to analyse subscribers only; 2
#' to analyse non-subscribers only; 3 to perform aged-based analyses
#' @param mf = 1/2 for male/female data (for subscriber=1 only)
#' @param data.dir = directory containing results from the C++ routines

plot_vgram <- function (city="nyc", measure="covar", osm=TRUE, nearfar=0,
                        subscriber=0, mf=0, data.dir="results/")
{
    require (gstat)
    require (sp)

    from <- c (FALSE, TRUE)
    ftxt <- c ("to", "from")
    cols <- c ("red", "lawngreen", "blue", "magenta")

    x11 (width=10, height=5)
    par (mfrow=c(1,2))

    mods <- c ("Sph", "Gau", "Exp", "Pow")
    ss <- array (NA, dim=c(4, 2))
    ipow <- which (mods == "Pow")

    funcs <- list ( sph <- function (x, cc, a) {
                            y <- cc * (1.5 * x / a - 0.5 * (x / a) ^ 3)
                            y [x > a] <- cc
                            return (y)  },
                    gau = function (x, cc, a) { 
                            cc * (1 - exp (-x ^ 2 / a ^ 2)) },
                    exp = function (x, cc, a) {
                            cc * (1 - exp (-x / a)) },
                    pow = function (x, cc, a) { cc * x ^ a  })

    for (i in 1:2) 
    {
        dat <- fit_gaussian (city=city, from=from [i], measure=measure, 
                                   nearfar=nearfar, subscriber=subscriber, mf=mf,
                                   data.dir=data.dir)
        if (osm)
            dists <- get_bike_data (city=city, measure=measure, 
                                    data.dir=data.dir)$d
        else
        {
            n <- dim (dat) [1]
            lons <- array (dat$x, dim=c(n, n))
            lats <- array (dat$y, dim=c(n, n))

            dlon = (lons - t (lons)) * pi / 180
            dlat = (lats - t (lats)) * pi / 180
            a = (sin (dlat / 2)) ^ 2 + 
                    cos (lats) * cos (t (lats)) * (sin (dlon / 2)) ^ 2
            dists = 6373 * 2 * atan2 (sqrt (a), sqrt (1 - a))
        }

        # TODO: Implement log-transformation of k-values with this condition:
        #if (shapiro.test (log10 (dat$k))$statistic >
        #    shapiro.test (dat$k)$statistic)

        k <- dat$k
        indx <- which (!is.na (dat$k))
        k <- k [indx]
        dists <- dists [indx, indx]
        n <- length (k)
        kmat <- array (k, dim=c(n, n))
        kdiff <- (kmat - t (kmat)) ^ 2 / 2

        dists <- dists [upper.tri (dists)]
        kdiff <- kdiff [upper.tri (kdiff)]
        indx <- order (dists)
        dists <- dists [indx]
        kdiff <- kdiff [indx]

        nbins <- 20
        len <- floor (length (dists) / nbins)
        dcut <- split(dists, ceiling(seq_along(dists) / len)) [1:nbins]
        kcut <- split(kdiff, ceiling(seq_along(kdiff) / len)) [1:nbins]

        d <- sapply (dcut, mean)
        k <- sapply (kcut, mean)
        dat <- data.frame (cbind (d, k))
        dfit <- seq (0, max(d), length.out=50)

        xlims <- c (0, max (d))
        ylims <- c (0, max (k))
        plot (d, k, pch=1, xlim=xlims, ylim=ylims,
              xlab="Distance", ylab="Semivariance")

        b <- NULL
        for (j in 1:length (funcs))
        {
            mod <- tryCatch (nls (k ~ funcs [[j]](d, cc, a),
                                data=dat, start=list(cc=2*mean(k), a=1)),
                             error=function (e) NULL)
            if (!is.null (mod))
            {
                coeffs <- summary (mod)$coefficients
                kfit <- do.call (funcs [[j]], 
                                 list(x=dfit, cc=coeffs [1], a=coeffs [2]))
                lines (dfit, kfit, col=cols [j])
                ss [j, i] <- sum (summary (mod)$residuals ^ 2)

                if (mods [j] == "Pow")
                    b <- formatC (coeffs [2], format="f", digits=2)
            }
        }

        best.mod <- mods [which.min (ss [,i])]
        legend (x="bottomright", lwd=2, pch=19, col=cols, bty="n", legend=mods)
        if (!is.null (b)) # can be null if no models fit
        {
            title (main=paste (toupper (ftxt [i]), ": ", best.mod, 
                               " b = ", b, sep=""))

            if (best.mod != "Pow")
            {
                powdiff <- 100 * (ss [ipow, i] / min (ss [,i], na.rm=TRUE) - 1)
                powdiff <- formatC (powdiff, format="f", digits=2)
                cat (toupper (ftxt [i]), ": SS for ", best.mod, 
                     " lower than Pow by ", powdiff, "%\n", sep="")
            }
        }
    }
    ss <- data.frame (ss * 1000)
    names (ss) <- ftxt
    row.names (ss) <- mods
    return (ss)
}
