#' plot_vgram
#'
#' Plots spatial semivariogram of k-values.
#'
#' Distances are in units of degrees, not kilometres.
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

plot_vgram <- function (city="nyc", measure="covar", nearfar=0,
                        subscriber=0, mf=0, data.dir="results/")
{
    require (gstat)
    require (sp)

    from <- c (FALSE, TRUE)
    ftxt <- c ("to", "from")
    cols <- c ("red", "lawngreen", "blue") # Sph, Exp, Pow fits to semivariance

    x11 (width=10, height=5)
    par (mfrow=c(1,2))

    mods <- c ("Pow", "Sph", "Exp")
    npars <- c (1, 2, 1)
    forms <- list (pow = function (s1, s2, r, x) s1 * x ^ r,
                   sph = function (s1, s2, r, x) {
                                y <- s1 + s2 * (1.5 * x / r - 0.5 * (x / r) ^ 3)
                                y [x > r] <- s1 + s2
                                return (y)  },
                   exp = function (s1, s2, r, x) s1 * (1 - exp (-x / r)))
    ss <- array (NA, dim=c(3, 2))

    for (i in 1:2) 
    {
        dat <- fit_gaussian (city=city, from=from [i], measure=measure, 
                                   nearfar=nearfar, subscriber=subscriber, mf=mf,
                                   data.dir=data.dir)
        coordinates (dat) = ~x+y
        if (shapiro.test (log10 (dat$k))$statistic >
            shapiro.test (dat$k)$statistic)
        {
            cat (toupper (ftxt [i]), ": k-values will be log-transformed\n")
            vgm <- variogram (log10 (k) ~ 1, dat)
        }
        else
            vgm <- variogram (k ~ 1, dat)
        # Isotropy can be confirmed with
        # > vgm <- variogram (...,map=TRUE,cutoff=0.03,width=0.001)
        # > plot (vgm, threshold=5)

        xlims <- c (0, max (vgm$dist))
        ylims <- c (0, max (vgm$gamma))
        plot (vgm$dist, vgm$gamma, pch=1, xlim=xlims, ylim=ylims,
              xlab="Distance", ylab="Semivariance")
        xfit <- seq (0, max (vgm$dist), length.out=100)

        for (j in 1:3)
        {
            if (npars [j] == 1)
                vgm.fit <- fit.variogram (vgm, model=vgm (1, mods [j], 1))
            else
                vgm.fit <- fit.variogram (vgm, model=vgm (1, mods [j], 1, 1))
            s1 <- vgm.fit$psill [1]
            s2 <- tail (vgm.fit$psill, n=1)
            r <- tail (vgm.fit$range, n=1)
            yfit <- do.call (forms [[j]], list (s1, s2, r, xfit))
            lines (xfit, yfit, col=cols[j])
            yfit <- do.call (forms [[j]], list (s1, s2, r, vgm$dist))
            ss [j, i] <- sum ((yfit - vgm$gamma) ^ 2)
            if (length (yfit) == 0)
                ss [j, i] <- NA

            if (mods [j] == "Pow")
                b <- formatC (vgm.fit$range, format="f", digits=3)
        }

        best.mod <- mods [which.min (ss [,i])]
        legend (x="bottomright", lwd=2, pch=19, col=cols, bty="n",
                legend=c("Pow","Sph","Exp"))
        title (main=paste (toupper (ftxt [i]), ": ", best.mod, 
                           " b = ", b, sep=""))
    }
    ss <- data.frame (ss * 1000)
    names (ss) <- ftxt
    row.names (ss) <- mods
    return (ss)
}
