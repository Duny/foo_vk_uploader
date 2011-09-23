#pragma once

class upload_thread : public threaded_process_callback
{
public:
	upload_thread (metadb_handle_list_cref p_items) : m_items (p_items) {}
    void start ();

private:
    metadb_handle_list_cref m_items;
};