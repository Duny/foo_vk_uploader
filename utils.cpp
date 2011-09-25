#include "stdafx.h"
//{"error":{"error_code":3,"error_msg":"Unknown method passed","request_params":[{"key":"oauth","value":"1"},{"key":"method","value":"sdfsf"},{"key":"access_token","value":"95e72c49dba8a93b95580d3ef395cf166f695ee95ea3c9e844b707319bbbc66"}]}}


void upload_items (metadb_handle_list_cref p_items)
{
	/*CURL *curl = curl_easy_init ();

	abort_callback_impl abort;
    file_ptr file;
	char *fileContent;
    filesystem::g_open (file, "d:\\Downloads\\04 - the_count_dance.mp3", filesystem::open_mode_read, abort);
    t_size fileSize = (t_size)file->get_size (abort);
    fileContent = new char[(unsigned int)fileSize];
    file->read (fileContent, fileSize, abort);

	struct curl_httppost* post = NULL, *last = NULL;
	CURLFORMcode curl_form = curl_formadd (&post, &last, 
		CURLFORM_COPYNAME, "act", CURLFORM_COPYCONTENTS, "load_audio", 
		CURLFORM_END);
	curl_form = curl_formadd (&post, &last, 
		CURLFORM_COPYNAME, "mid", CURLFORM_COPYCONTENTS, "856287", 
		CURLFORM_END);
	curl_form = curl_formadd (&post, &last, 
		CURLFORM_COPYNAME, "hash", CURLFORM_COPYCONTENTS, "ce17eccdc6cf4e63fdd2f7081e6aa93d", 
		CURLFORM_END);
		curl_form = curl_formadd (&post, &last, 
		CURLFORM_COPYNAME, "rhash", CURLFORM_COPYCONTENTS, "f943e016e1217b6e9b23cdb795ccd594", 
		CURLFORM_END);
	curl_form = curl_formadd (&post, &last,
		CURLFORM_COPYNAME, "file",
		CURLFORM_BUFFER, "04 - the_count_dance.mp3",
		CURLFORM_CONTENTTYPE, "audio/mpeg",
		CURLFORM_BUFFERPTR, fileContent,
        CURLFORM_BUFFERLENGTH, fileSize,
		CURLFORM_END);
	

	struct curl_slist *slist = NULL;
	slist = curl_slist_append (slist, "Expect:");
	slist = curl_slist_append (slist, "Cookie: remixsid=8ae9eb7954b84f94a3c4b641da4d5d391550492fdc28a1e24b5117c7db7e");

	curl_easy_setopt (curl, CURLOPT_HTTPHEADER, slist);
	curl_easy_setopt (curl, CURLOPT_URL, "http://cs5029.vkontakte.ru/upload.php");
	curl_easy_setopt (curl, CURLOPT_HTTPPOST, post);
	curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, readHeaders);

	CURLcode res = curl_easy_perform (curl);

	delete [] fileContent;
	curl_formfree (post);
	curl_slist_free_all (slist);
	curl_easy_cleanup (curl);*/

    //vk
}