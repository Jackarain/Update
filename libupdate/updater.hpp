//
// updater.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2011 Jack (jack.wgm@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// * $Id: updater.hpp 49 2011-07-15 03:00:34Z jack $
//

#ifndef __UPDATER_H__
#define __UPDATER_H__

#pragma once

#include <boost/function.hpp>

class updater_impl;

class updater
{
public:
	// 文件检查回调.
	typedef boost::function<void (std::string file, int count, int index)> fun_check_files_callback;
	// 更新文件回调.
	typedef boost::function<void (std::string file, int count, int index)> fun_update_files_process;
	// 文件下载回调.
	typedef boost::function<void (std::string file, int count, int index, int total_size, int total_read_bytes,
		int file_size, int read_bytes)> fun_down_load_callback;

	enum result_type {
		st_error,						// 出错.
		st_updating,					// 更新中.
		st_succeed,						// 更新成功.
		st_no_need_update,			// 不需要更新.
		st_unable_to_connect,		// 无法连接到更新服务器.
		st_invalid_http_response,	// http错误的返回.
		st_open_file_failed			// 打开文件失败.
	};

public:
	updater();
	~updater();

public:

	// @ 开始更新.
	// @ url 是xml所在的地址.
	// @ fc	setup_path所指的目录下的文件检查, 如果文件完整, 则不需要下载更新.
	// @ dl	下载回调函数, 回调时返回当前下载的文件名, 总文件数	, 当前下载的
	//			文件索引, 整个更新的所有文件大小, 当前已经下载的所有数据大小(包
	//			括已经下载过的文件), 当前下载的文件的大小, 当前文件已下载的数据
	//			大小.
	// @ cf	文件检查进度回调, 当更新下载完成后, 检查下载的文件的进
	//			度.
	// @ uf	文件更新进度回调, 当文件检查完成后, 就开始进行更新.
	// @ setup_path 更新位置.
	bool start(const std::string& url, fun_check_files_callback fc, fun_down_load_callback dl,
		fun_check_files_callback cf, fun_update_files_process uf, std::string setup_path);

	// @ 检查是否需要更新, 需要更新返回true, 不需要返回false.
	bool check_update(const std::string& url, const std::string& setup_path);

	// @ 停止更新.
	void stop();

	// @ 暂停更新.
	void pause();

	// @ 继续更新.
	void resume();

	// @ 更新最后结果.
	result_type result();

private:
	updater_impl *m_updater;
};

#endif // __UPDATER_H__
