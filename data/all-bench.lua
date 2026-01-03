-- data/all-bench.lua
-- 综合基准：常量折叠、函数内联、作用域对比、分配下沉、NYI、FFI
-- 用法:
--   luajit all-bench.lua
--   luajit -jdump all-bench.lua > dump.txt
local has_ffi, ffi = pcall(require, "ffi")
local DEFAULT_N = 1e7
local DEFAULT_REPEATS = 5
local DEFAULT_WARMUP = 2

-- ---------- 统计工具函数 ----------
local function mean(t)
	local s = 0

	for _, v in ipairs(t) do
		s = s + v
	end

	return s / #t
end

local function median(t)
	table.sort(t)

	local n = #t

	if n % 2 == 1 then
		return t[math.floor((n + 1) / 2)]
	end

	return (t[math.floor(n / 2)] + t[math.floor(n / 2) + 1]) / 2
end

local function stddev(t)
	local m = mean(t)
	local s = 0

	for _, v in ipairs(t) do
		s = s + (v - m) ^ 2
	end

	return math.sqrt(s / #t)
end

local function bench(fn, name, opts)
	opts = opts or {}

	local n = opts.n or DEFAULT_N
	local repeats = opts.repeats or DEFAULT_REPEATS
	local warmup = opts.warmup or DEFAULT_WARMUP
	local lastres = nil

	for i = 1, warmup do
		lastres = fn(n)
	end

	local times = {}

	for i = 1, repeats do
		local t0 = os.clock()
		local res = fn(n)
		local t1 = os.clock()

		times[#times + 1] = t1 - t0
		lastres = res
	end

	print(string.format("%-30s runs=%d  mean=%.6f  med=%.6f  std=%.6f", name, #times, mean(times), median(times), stddev(times)))

	return times
end

-- ---------- 1. 常量折叠测试 (Constant Folding) ----------
local function make_not_folded()
	return function(N)
		local s = 0

		for i = 1, N do
			s = s + (1 + 2 * 3)
			s = s + ((4 + 5) / 3)
			s = s + (2 ^ 3 + 4 ^ 2)
			s = s + (10 - 3 - 2)
		end

		return s
	end
end

local function make_folded_literal()
	return function(N)
		local s = 0

		for i = 1, N do
			s = s + 7
			s = s + 3
			s = s + 24
			s = s + 5
		end

		return s
	end
end

local function make_command_folded()
	return function(N)
		local s = 0

		for i = 1, N do
			s = s + 10
		end

		return s
	end
end

local function make_command_not_folded()
	return function(N)
		local s = 0

		for i = 1, N do
			s = s + 1
			s = s + 2
			s = s + 3
			s = s + 4
		end

		return s
	end
end

local function make_math_folded()
	return function(N)
		local s = 0

		for i = 1, N do
			s = s + 4
			s = s + 1
		end

		return s
	end
end

local function make_math_not_folded()
	return function(N)
		local s = 0

		for i = 1, N do
			s = s + math.sqrt(16)
			s = s + math.sin(math.pi / 2)
		end

		return s
	end
end

local function make_if_assign()
	return function(N)
		local s = 0

		for i = 1, N do
			local v

			if i % 2 == 0 then
				v = 1
			else
				v = 2
			end

			s = s + v
		end

		return s
	end
end

local function make_ternary_assign()
	return function(N)
		local s = 0

		for i = 1, N do
			local v = (i % 2 == 0) and 1 or 2

			s = s + v
		end

		return s
	end
end

local function make_for_unroll()
	return function(N)
		local s = 0

		for i = 1, N do
			s = s + 1
			s = s + 1
			s = s + 1
			s = s + 1
		end

		return s
	end
end

local function make_for_no_unroll()
	return function(N)
		local s = 0

		for i = 1, N do
			for j = 1, 4 do
                s = s + 1
            end
		end

		return s
	end
end

-- ---------- 2. 函数内联测试 (Function Inlining) ----------
local function make_fn_inline()
	local function add_inline(a, b)
		return a + b + 1
	end

	return function(N)
		local s = 0

		for i = 1, N do
			s = s + add_inline(1, 2)
		end

		return s
	end
end

local function make_fn_noinline_const()
	local up = 1

	local function add_noinline(a, b)
		return a + b + up
	end

	return function(N)
		local s = 0

		for i = 1, N do
			s = s + add_noinline(1, 2)
		end

		return s
	end
end

-- ---------- 3. 全局/局部/缓存读测试 (Scope & Cache) ----------
GLOBAL_CONST = 123

local LOCAL_CONST = 123

local function make_test_global()
	return function(N)
		local s = 0

		for i = 1, N do
			s = s + GLOBAL_CONST
		end

		return s
	end
end

local function make_test_local()
	return function(N)
		local s = 0

		for i = 1, N do
			s = s + LOCAL_CONST
		end

		return s
	end
end

local function make_test_literal()
	return function(N)
		local s = 0

		for i = 1, N do
			s = s + 123
		end

		return s
	end
end

local function make_test_global_read_cache()
	return function(N)
		local s = 0

		for i = 1, N do
			local v = GLOBAL_CONST

			s = s + v
		end

		return s
	end
end

-- 临时表分配：每次循环都新建表并读取字段（会分配）
local function make_test_temp_table_alloc()
	return function(N)
		local s = 0

		for i = 1, N do
			local t = {
				x = i,
				y = i + 1
			}

			s = s + t.x + t.y
		end

		return s
	end
end

-- 直接使用局部值（无表分配）
local function make_test_direct_values()
	return function(N)
		local s = 0

		for i = 1, N do
			local x = i
			local y = i + 1

			s = s + x + y
		end

		return s
	end
end

-- 重用单个表（避免在循环中分配）
local function make_test_reused_table()
	local t = {
		x = 0,
		y = 0
	}

	return function(N)
		local s = 0

		for i = 1, N do
			t.x = i
			t.y = i + 1
			s = s + t.x + t.y
		end

		return s
	end
end

-- ---------- 5. NYI 与迭代器测试 (NYI Traps) ----------
local function make_test_nyi_pairs()
	local t = {
		a = 1,
		b = 2
	}

	return function(N)
		local s = 0

		for i = 1, N do
			for k, v in pairs(t) do
				s = s + v
			end
		end

		return s
	end
end

local function make_test_jit_ipairs()
	local t = {1, 2}

	return function(N)
		local s = 0

		for i = 1, N do
			for k, v in ipairs(t) do
				s = s + v
			end
		end

		return s
	end
end

local function make_test_numeric_for()
	local t = {1, 2}

	return function(N)
		local s = 0

		for i = 1, N do
			for j = 1, #t do
				s = s + t[j]
			end
		end

		return s
	end
end

-- ---------- 任务注册列表 ----------
local tests = {
	{
		name = "const_expr_not_folded",
		fn = make_not_folded()
	},
	{
		name = "const_literal_folded",
		fn = make_folded_literal()
	},
	{
		name = "command_not_folded",
		fn = make_command_not_folded()
	},
	{
		name = "command_folded",
		fn = make_command_folded()
	},
	{
		name = "math_not_folded",
		fn = make_math_not_folded()
	},
	{
		name = "math_folded",
		fn = make_math_folded()
	},
	{
		name = "if_assign",
		fn = make_if_assign()
	},
	{
		name = "ternary_assign",
		fn = make_ternary_assign()
	},
    {
        name = "for_no_unroll",
        fn = make_for_no_unroll()
    },
    {
        name = "for_unroll",
        fn = make_for_unroll()
    },
	{
		name = "fn_inline",
		fn = make_fn_inline()
	},
	{
		name = "fn_noinline_const_up",
		fn = make_fn_noinline_const()
	},
	{
		name = "global_const",
		fn = make_test_global()
	},
	{
		name = "local_const",
		fn = make_test_local()
	},
	{
		name = "literal_const",
		fn = make_test_literal()
	},
	{
		name = "global_read_cached_local",
		fn = make_test_global_read_cache()
	},
	{
		name = "temp_table_alloc",
		fn = make_test_temp_table_alloc()
	},
	{
		name = "direct_values_no_alloc",
		fn = make_test_direct_values()
	},
	{
		name = "reused_table_no_alloc",
		fn = make_test_reused_table()
	},
	{
		name = "nyi_pairs",
		fn = make_test_nyi_pairs()
	},
	{
		name = "jit_ipairs",
		fn = make_test_jit_ipairs()
	},
	{
		name = "numeric_for",
		fn = make_test_numeric_for()
	}
}

-- ---------- 7. FFI 性能测试 (FFI Efficiency) ----------
if has_ffi then
	-- FFI 更详细对比测试（放到 has_ffi 分支内）
	ffi.cdef[[
typedef struct { double x, y; } point_t;
]]

	-- 1) FFI 重用 struct（单个 struct 在循环中重写字段）
	local function make_test_ffi_struct_reused()
		local p = ffi.new("point_t")

		return function(N)
			local s = 0.0

			for i = 1, N do
				p.x = i
				p.y = i + 1
				s = s + p.x + p.y
			end

			return s
		end
	end

	-- 2) 等价的 Lua 表重用（避免每次分配）
	local function make_test_lua_table_reused()
		local p = {
			x = 0.0,
			y = 0.0
		}

		return function(N)
			local s = 0.0

			for i = 1, N do
				p.x = i
				p.y = i + 1
				s = s + p.x + p.y
			end

			return s
		end
	end

	-- 3) FFI 数组访问（用 ctype 数组，连续内存）
	local function make_test_ffi_array()
		local M = 1024
		local a = ffi.new("double[?]", M)

		return function(N)
			local s = 0.0

			for i = 1, N do
				local idx = (i - 1) % M + 1

				a[idx - 1] = i -- 注意 C 数组 0-based
				s = s + a[idx - 1]
			end

			return s
		end
	end

	-- 4) Lua 数组（table）访问对照
	local function make_test_lua_array()
		local M = 1024
		local a = {}

		for i = 1, M do
			a[i] = 0.0
		end

		return function(N)
			local s = 0.0

			for i = 1, N do
				local idx = (i - 1) % M + 1

				a[idx] = i
				s = s + a[idx]
			end

			return s
		end
	end

	-- 把上面四项加入 tests 列表（仅在 has_ffi 时）
	table.insert(tests, {
		name = "ffi_struct_reused",
		fn = make_test_ffi_struct_reused()
	})
	table.insert(tests, {
		name = "lua_table_reused",
		fn = make_test_lua_table_reused()
	})
	table.insert(tests, {
		name = "ffi_array_access",
		fn = make_test_ffi_array()
	})
	table.insert(tests, {
		name = "lua_array_access",
		fn = make_test_lua_array()
	})
end

local function run_all(opts)
	local n = opts.n or DEFAULT_N

	print(string.format("Benchmark N=%d repeats=%d warmup=%d", n, opts.repeats, opts.warmup))

	for _, t in ipairs(tests) do
		bench(t.fn, t.name, opts)
	end
end

-- ---------- CLI 解析 ----------
local args = {...}
local opts = {
	n = DEFAULT_N,
	repeats = DEFAULT_REPEATS,
	warmup = DEFAULT_WARMUP
}

for i = 1, #args do
	if args[i] == "-n" and args[i + 1] then
		opts.n = tonumber(args[i + 1])
	end

	if args[i] == "-r" and args[i + 1] then
		opts.repeats = tonumber(args[i + 1])
	end

	if args[i] == "-w" and args[i + 1] then
		opts.warmup = tonumber(args[i + 1])
	end
end

run_all(opts)
