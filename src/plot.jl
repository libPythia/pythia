using CSV, DataFrames, Query, Statistics, Gadfly, Cairo, Fontconfig

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

function plotData(path, pdf = "")
    df = loadData(path)
    sizes = listEntries(df, :trace_size)
    alphas = listEntries(df, :alpha_size)

    df_mean = @from i in df begin
        @orderby i.trace_size, i.alpha_size
        @group i by i.trace_size, i.alpha_size into g
        @select {trace_size = g.trace_size[1],
                 alpha_size=g.alpha_size[1],
                 duration=mean(g.duration) / 1e9,
                 node_count=mean(g.node_count),
                 insertion=mean(g.duration)/g.trace_size[1]/1e3,
                 node_by_size=mean(g.node_count)/g.trace_size[1]/1e3}
        @collect DataFrame
    end

    if (length(sizes) > length(alphas)) # Duration in function of size
        p1 = plot(df_mean,
             x=:trace_size,
             Guide.xlabel("Trace size"),
             y=:duration,
             Guide.ylabel("Factorization duration (s)"),
             color=:alpha_size,
             Guide.colorkey(title="Alphabet size"),
             Scale.color_discrete(),
             Geom.line
            )
        p2 = plot(df_mean,
             x=:trace_size,
             Guide.xlabel("Trace size"),
             y=:insertion,
             Guide.ylabel("Insertion duration (μs)"),
             color=:alpha_size,
             Guide.colorkey(title="Alphabet size"),
             Scale.color_discrete(),
             Geom.line
            )
        p3 = plot(df_mean,
             x=:trace_size,
             Guide.xlabel("Trace size"),
             y=:node_count,
             Guide.ylabel("Nodes count"),
             color=:alpha_size,
             Guide.colorkey(title="Alphabet size"),
             Scale.color_discrete(),
             Geom.line
             # Geom.smooth(method=:loess, smoothing=0.1)
            )
        p4 = plot(df_mean,
            x=:trace_size,
            Guide.xlabel("Trace size"),
            y=:node_by_size,
            Guide.ylabel("Node count / trace size"),
            color=:alpha_size,
            Guide.colorkey(title="Alphabet size"),
            Scale.color_discrete(),
            Geom.line
           )
        if (pdf == "")
            display(p1)
            display(p2)
            display(p3)
            display(p4)
        else
            p1 |> PDF(pdf * "_duration_trace.pdf")
            p2 |> PDF(pdf * "_insertion_trace.pdf")
            p3 |> PDF(pdf * "_node_trace.pdf")
            p4 |> PDF(pdf * "_compression_trace.pdf")
        end
    else # Duration in function of alphabet size
        p1 = plot(df_mean,
             x=:alpha_size,
             Guide.xlabel("Alphabet size"),
             y=:duration,
             Guide.ylabel("Factorization duration (s)"),
             color=:trace_size,
             Guide.colorkey(title="Trace size"),
             Scale.color_discrete(),
             Geom.line
             # Geom.smooth(method=:loess, smoothing=0.1)
            )
        p2 = plot(df_mean,
             x=:alpha_size,
             Guide.xlabel("Alphabet size"),
             y=:insertion,
             Guide.ylabel("Insertion duration (μs)"),
             color=:trace_size,
             Guide.colorkey(title="Trace size"),
             Scale.color_discrete(),
             Geom.line
             # Geom.smooth(method=:loess, smoothing=0.1)
            )
        p3 = plot(df_mean,
             x=:alpha_size,
             Guide.xlabel("Alphabet size"),
             y=:node_count,
             Guide.ylabel("Nodes count"),
             color=:trace_size,
             Guide.colorkey(title="Trace size"),
             Scale.color_discrete(),
             Geom.line
             # Geom.smooth(method=:loess, smoothing=0.1)
            )
        p4 = plot(df_mean,
            x=:alpha_size,
            Guide.xlabel("Alphabet size"),
            y=:node_by_size,
            Guide.ylabel("Node count / trace size"),
            color=:trace_size,
            Guide.colorkey(title="Trace size"),
            Scale.color_discrete(),
            Geom.line
           )
        if (pdf == "")
            display(p1)
            display(p2)
            display(p3)
            display(p4)
        else
            p1 |> PDF(pdf * "_duration_alpha.pdf")
            p2 |> PDF(pdf * "_insertion_alpha.pdf")
            p3 |> PDF(pdf * "_node_alpha.pdf")
            p4 |> PDF(pdf * "_compression_alpha.pdf")
        end
    end
end
