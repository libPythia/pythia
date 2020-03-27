using CSV, DataFrames, Query, Statistics, Gadfly

loadData(path) = DataFrame(CSV.File(path))

listEntries(df::DataFrame, sym) = @from i in df begin
    @group i by i[sym] into g
    @select key(g)
    @collect
end

computeInsertionTime(df::DataFrame) = @from i in df begin
    @select {i..., insertion_time=i.duration/i.trace_size}
    @collect DataFrame
end

function plotData(path)
    df = loadData(path)
    sizes = listEntries(df, :trace_size)
    alphas = listEntries(df, :alpha_size)

    df_mean = @from i in df begin
        @group i by i.trace_size, i.alpha_size into g
        @select {trace_size = g.trace_size[1], alpha_size=g.alpha_size[1], duration=mean(g.duration) / 1e9, insertion=mean(g.duration)/g.trace_size[1]/1e3}
        @collect DataFrame
    end

    if (length(sizes) > length(alphas)) # Duration in function of size
        p1 = plot(df_mean,
             x=:trace_size,
             Guide.xlabel("Trace size"),
             y=:duration,
             Guide.ylabel("Factorization duration (seconds)"),
             color=:alpha_size,
             Guide.colorkey(title="Alphabet size"),
             Scale.color_discrete(),
             Geom.line
            )
        p2 = plot(df_mean,
             x=:trace_size,
             Guide.xlabel("Trace size"),
             y=:insertion,
             Guide.ylabel("Average insersion duration (milliseconds)"),
             color=:alpha_size,
             Guide.colorkey(title="Alphabet size"),
             Scale.color_discrete(),
             Geom.line
            )
        display(p1)
        display(p2)
    else # Duration in function of alphabet size
        plot(df_mean,
             x=:alpha_size,
             Guide.xlabel("Alphabet size"),
             y=:duration,
             Guide.ylabel("Factorization duration (seconds)"),
             color=:trace_size,
             Guide.colorkey(title="Trace size"),
             Scale.color_discrete(),
             Geom.line
             # Geom.smooth(method=:loess, smoothing=0.1)
            )
    end
end
