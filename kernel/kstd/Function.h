/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

namespace kstd {
	template<typename RetT, typename... ArgTs>
	class FunctionWrapperBase {
	public:
		virtual ~FunctionWrapperBase() {}
		virtual RetT invoke(ArgTs...) = 0;
	};

	template<typename RawFuncT, typename RetT, typename... ArgTs>
	class FunctionWrapper : public FunctionWrapperBase<RetT, ArgTs...> {
	public:
		FunctionWrapper(RawFuncT&& func): m_func(func) {}
		RetT invoke(ArgTs... args) override { return m_func(args...); }

	private:
		RawFuncT m_func;
	};

	template<typename>
	class Function;

	template<typename RetT, typename... ArgTs>
	class Function<RetT(ArgTs...)> {
	public:
		using FuncT = RetT(ArgTs...);

		Function() = default;

		template<typename RawFuncT>
		Function(RawFuncT&& raw_func):
			m_funcwrapper(new FunctionWrapper<RawFuncT, RetT, ArgTs...>(static_cast<RawFuncT&&>(raw_func))) {}

		Function& operator=(FuncT func) {
			m_funcwrapper = func;
			return *this;
		}

		RetT operator()(ArgTs... args) {
			return m_funcwrapper->invoke(args...);
		}

	private:
		kstd::Arc<FunctionWrapperBase<RetT, ArgTs...>> m_funcwrapper;
	};
}