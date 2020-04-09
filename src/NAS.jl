using CSV, DataFrames, Query, Statistics, Gadfly

NAS_root = "NAS_PB"
NAS_dir = "$NAS_root/compat_trace"
tmp_root = "/tmp/eta_NAS"

countLine(file_name) = open(file_name, "r") do file; file |> eachline |> collect |> length end
countInt(file_name) = open(file_name, "r") do file;
    (readline(file) |> line -> split(line, " ", keepempty=false) |> collect |> length
end


function runBench()
    run(`ninja -C rel`)
    open("$NAS_root/out.dat", "w") do output
        write(output, "kernel thread alpha_size trace_size insertion_duration factorization_duration\n")

        for kernel in readdir(NAS_dir)
            mkpath("$tmp_root/$kernel")
            println(kernel)
            for dict_name in filter(readdir("$NAS_dir/$kernel")) do file_name
                    startswith(file_name, "dict_")
                end

                thread_name = replace(replace(dict_name, r"dict_" => ""), r"\.dat" => "")
                trace_name = "events_$(thread_name).dat"

                dict_path = "$NAS_dir/$kernel/$dict_name"
                trace_path = "$NAS_dir/$kernel/$trace_name"

                dict_size = countLine(dict_path)
                trace_size = countInt(trace_path)

                insertions = Vector{Float64}()
                factorizations = Vector{Float64}()

                map(1:30) do i
                    tmp_path = "/tmp/eta_NAS/$kernel/$thread_name.data"
                    run(pipeline(`rel/bench -f $trace_path`, stdout=tmp_path))
                    df = DataFrame(CSV.File(tmp_path))

                    push!(insertions, @from i in df begin
                        @select i.last_insert_duration
                        @collect mean
                    end)

                    push!(factorizations, @from i in df begin
                        @select i.full_factorization_duration
                        @collect maximum
                    end)
                end

                insertion = mean(insertions)
                factorization = mean(factorizations)

                println("- $dict_size $trace_size $insertion $factorization")

                write(output, "$kernel $thread_name $dict_size $trace_size $insertion $factorization\n")
            end
        end
    end
end

