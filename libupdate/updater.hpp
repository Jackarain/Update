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

#include <boost/function.hpp>
class updater_impl;

#pragma once


class updater
{
public:
	typedef boost::function<void (std::string file, 
		int count, int index)> fun_check_files_callback;
	typedef boost::function<void (std::string file, 
		int count, int index)> fun_update_files_process;
	typedef boost::function<void (std::string file, int count, int index, 
		int total_size, int total_read_bytes,
		int file_size, int read_bytes)> fun_down_load_callback;

public:
	updater();
	~updater();

public:

	// @ 开始更新.
	// @ url 是xml所在的地址.
	// @ dl	下载回调函数, 回调时返回当前下载的文件名, 总文件数
	//			, 当前下载的文件索引, 整个更新的所有文件大小, 当前
	//			已经下载的所有数据大小(包括已经下载过的文件), 当前
	//			下载的文件的大小, 当前文件已下载的数据大小.
	// @ cf	文件检查进度回调, 当更新下载完成后, 检查下载的文件的进
	//			度.
	// @ uf	文件更新进度回调, 当文件检查完成后, 就开始进行更新.
	// @ setup_path 更新位置.
	bool start(const std::string& url, fun_down_load_callback dl, 
		fun_check_files_callback cf, fun_update_files_process uf, std::string setup_path);

	// @ 停止更新.
	void stop();

	// @ 暂停更新.
	void pause();

	// @ 继续更新.
	void resume();

private:
	updater_impl *m_updater;
};

#endif // __UPDATER_H__
