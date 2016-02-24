/**
cybozu_get_file

Copyright (c) 2016 yumetodo

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <locale>
#include <cstdint>
#include <codecvt>
#include <regex>
#include "arithmetic_convert.hpp"
#include "input.hpp"
struct info {
	std::wstring cid;
	std::wstring filename;
};
template<typename char_type1, typename char_type2>
void download(const std::basic_string<char_type1>& url, const std::basic_string<char_type2>& out_filename, const char* cokie, char_cvt::char_enc type)
{
	const char* const header_base = R"(--header "Host: cybozulive.com" --header "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:44.0) Gecko/20100101 Firefox/44.0" --header "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" --header "Accept-Language: ja,en-US;q=0.7,en;q=0.3" --header "Connection: keep-alive")";
	//Issue : won't fix : Unicode string shold be normarized. Because some charactor(ex. U+3099) cannot convert to shift-jis.
	const std::string cmd = std::string("curl") + ' ' + header_base + R"( --header "Cookie: )" + cokie + R"(" -L ")" + char_cvt::to_string(url, type) + R"(" -o ")" + char_cvt::to_string(out_filename, type) + "\"";
	std::cout << "downloading " << char_cvt::to_string(out_filename, type) << std::endl;
	std::system(cmd.c_str());//exec
	std::this_thread::sleep_for(std::chrono::milliseconds(2500));//岡崎市立中央図書館事件を忘れることなかれ
}
std::wstring make_dl_file_utl(const std::wstring& group_id, const std::wstring& cid)
{
	return L"https://cybozulive.com/" + group_id + L"/gwCabinet/downloadFileDirect?cid=" + cid;
}
std::wstring make_dl_file_name(const std::wstring& base_name, const std::wstring cid)
{
	if (std::ifstream(base_name)) {
		//file exists
		const auto pos = base_name.find_last_of(L'.');
		return base_name.substr(0, pos) + L'_' + cid + base_name.substr(pos);//cidを付加してconflict防止
	}
	return base_name;
}
std::unordered_map<std::wstring, std::vector<info>> load_cid_list(const char* filename)
{
	std::wifstream file(filename);
	file.imbue(std::locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t>));
	std::unordered_map<std::wstring, std::vector<info>> re;
	std::wstring pre;
	for (std::wstring buf; std::getline(file, buf);) {
		std::wsmatch m; // match_results
		std::regex_search(buf, m, std::wregex(LR"(folder_id:\d+ cid:(\d+) filename:(.+))"));
		if (m.size() < 1) {
			pre = std::move(buf);
		}
		else if (m.size() == 3 && !pre.empty()) {
			re[pre].push_back(info{ std::move(m[1].str()), std::move(m[2].str()) });
		}
	}
	return re;
}
int main() {
	const wchar_t* const group_id = L"";
	const char* const cokie = "";
	//input char encode type
	const auto type = static_cast<char_cvt::char_enc>(input("char encode (1=UTF-8, 2=Shift-JIS)", 2, 1));
	const auto cid_list = load_cid_list("cid_list.txt");
	for (const auto& cid : cid_list) {
		std::system(("mkdir " + char_cvt::to_string(cid.first, type)).c_str());
		for (const auto& info : cid.second) {
			download(make_dl_file_utl(group_id, info.cid), make_dl_file_name(cid.first + L'\\' + info.filename, info.cid), cokie, type);
		}
	}
}
