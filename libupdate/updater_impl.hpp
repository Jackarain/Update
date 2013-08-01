//
// updater_impl.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2011 Jack (jack.wgm@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// * $Id: updater_impl.hpp 49 2011-07-15 03:00:34Z jack $
//

#ifndef __UPDATER_IMPL_H__
#define __UPDATER_IMPL_H__

#include "url.hpp"
#include "md5.hpp"
#include "tinyxml.h"
#include "updater.hpp"

#include <stdio.h>
#include <string.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>


#include <boost/asio.hpp>
using boost::asio::ip::tcp;

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#include <boost/algorithm/string.hpp>


#pragma once

class updater_impl
{
public:
	typedef boost::function<void (std::string file, 
		int count, int index)> fun_check_files_callback;
	typedef boost::function<void (std::string file, 
		int count, int index)> fun_update_files_process;
	typedef boost::function<void (std::string file, int count, int index, 
		int total_size, int total_read_bytes, int file_size, int read_bytes)> fun_down_load_callback;

	typedef struct
	{
		std::string name;             // 文件名.
		std::string md5;              // 文件的MD5值.
		std::string filehash;         // 压缩文件的HASH.
		std::string url;              // 下载的url, 默认为空.
		std::string command;          // 命令.
		std::string compress;         // 压缩选项.
		bool check;							// 文件检查选项, 如果为ture, 则表示不比较MD5, 只当文件不存在时才更新.
		boost::uint32_t size;         // 大小.
	} xml_node_info;

public:
	updater_impl(void);
	virtual ~updater_impl(void);

public:
	bool start(const std::string& url, fun_check_files_callback fc, fun_down_load_callback dl,
		fun_check_files_callback cf, fun_update_files_process uf, std::string setup_path);
	bool check_update(const std::string& url, const std::string& setup_path);
	void stop();
	void pause();
	void resume();
	updater::result_type result();

protected:
	// 更新.
	void update_files();
	bool parser_xml_file(const std::string& file);

	// 文件下载.
	bool file_down_load(const std::string& u, const std::string& file,
		xml_node_info& info, const std::string& extera_header = "");
	bool file_down_load_by_avhttp(const std::string& u, const std::string& file,
		xml_node_info& info, const std::string& extera_header = "");
	void down_load_callback(std::string file, int count, int index, 
		int total_size, int total_read_bytes, int file_size, int read_bytes);

	std::string make_http_last_modified(const std::string& file);
	bool parser_http_last_modified(const std::string& str, struct tm* time);

private:
	fun_check_files_callback m_setup_file_check;
	fun_down_load_callback m_down_load_fun;
	fun_check_files_callback m_fun_check_files;
	fun_update_files_process m_update_files_fun;
	bool m_is_downloading;
	std::string m_url;
	std::string m_setup_path;
	boost::thread m_update_thrd;
	std::map<std::string, xml_node_info> m_update_file_list;
	std::map<std::string, xml_node_info> m_need_update_list;
	boost::uint64_t m_upfile_total_size;
	int m_current_index;
	int m_total_read_bytes;
	boost::asio::io_service m_io_service;
	boost::weak_ptr<tcp::socket> m_sock;
	bool m_abort;
	bool m_paused;
	updater::result_type m_result;
};

#endif // __UPDATER_IMPL_H__
