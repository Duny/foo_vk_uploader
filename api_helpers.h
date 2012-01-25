#ifndef _FOO_VK_UPLOADER_API_HELPERS_H_
#define _FOO_VK_UPLOADER_API_HELPERS_H_

namespace vk_uploader
{
    class NOVTABLE api_method_base
    {
    public:
        api_method_base () : m_aborted (false) {}

        // Does actual call of vk api method
        bool call (abort_callback & p_abort = abort_callback_impl ())
        {
            m_aborted = false;
            m_error.reset ();

            try { run (p_abort); }
            catch (const exception_aborted &) { m_aborted = true; }
            catch (const pfc::exception & e) { m_error = e.what (); }

            return m_error.is_empty () && !m_aborted;
        }

        bool aborted () const { return m_aborted; }
        const char * get_error () const { return m_error; }

    private:
        virtual void run (abort_callback & p_abort) = 0;
        bool m_aborted; // Call was aborted by user
    protected:
        pfc::string8 m_error;
    };

    // helper implementation of some apis
    namespace vk_api
    {
        namespace audio
        {
            namespace albums
            {
                // Reads a list of user albums (from vk.com profile)
                // Represents as list of (album_name, album_id) pairs
                class get : public api_method_base,
                            public pfc::list_t<audio_album_info>
                {
                    void run (abort_callback & p_abort) override;
                };


                // Creates new album in user profile
                class add : public api_method_base
                {
                    void run (abort_callback & p_abort) override;
                    audio_album_info m_new_album;
                public:
                    explicit add (const pfc::string_base & title) : m_new_album (make_tuple (title.get_ptr (), 0)) {}

                    operator audio_album_info const & () const  { return m_new_album; }
                };


                // Deletes album and all of it contents (!) from user profile
                class del : public api_method_base
                {
                    void run (abort_callback & p_abort) override;
                    t_vk_album_id m_id_to_delete;
                public:
                    del (t_vk_album_id id) : m_id_to_delete (id) {}
                };


                // Renames album in user profile
                class ren : public api_method_base
                {
                    void run (abort_callback & p_abort) override;
                    t_vk_album_id m_album_id;
                    pfc::string8  m_new_title;
                public:
                    ren (t_vk_album_id album_id, const pfc::string_base & p_new_title)
                        : m_album_id (album_id), m_new_title (p_new_title) {}
                };
            } // namespace albums


            // Moves vk audio track to specified album
            class move_to_album : public api_method_base
            {
                void run (abort_callback & p_abort) override;
                t_vk_audio_id m_audio_id;
                t_vk_album_id m_album_id;
            public:
                move_to_album (t_vk_audio_id audio_id, t_vk_album_id album_id)
                    : m_audio_id (audio_id), m_album_id (album_id) {}
            };


            // Queries url for posting upload to
            class get_upload_server : public api_method_base
            {
                void run (abort_callback & p_abort) override;
                pfc::string8 m_url;
            public:
                operator const char* () const { return m_url.get_ptr (); }
            };


            // Saves newly uploaded audio file in users profile
            class save : public api_method_base
            {
                void run (abort_callback & p_abort) override;
                response_json_ptr m_result;
                t_vk_audio_id m_id; // Id of newly uploaded mp3 file
            public:
                // Answer from vk.com server returned after file was upload
                save (const pfc::string_base & p_answer) : m_result (p_answer) {}

                t_vk_audio_id get_id () const { return m_id; }
            };

            // Edits artist/title fields of the track m_id
            class edit : public api_method_base
            {
                void run (abort_callback & p_abort) override;
                metadb_handle_ptr m_track;
                t_vk_audio_id m_aid;
            public:
                // Answer from vk.com server returned after file was upload
                edit (const metadb_handle_ptr & p_track, t_vk_audio_id aid) : m_track (p_track), m_aid (aid) {}
            };

        } // namespace audio


        namespace wall
        {
            // Makes post on wall this optimal message and audios attached
            class post : public api_method_base
            {
                void run (abort_callback & p_abort) override;
                pfc::string8 m_message;
                const pfc::list_t<t_vk_audio_id> & m_audio_ids;
            public:
                post (const pfc::string_base & p_msg, const pfc::list_t<t_vk_audio_id> & p_audio_ids)
                    : m_message (p_msg), m_audio_ids (p_audio_ids) {}
            };
        }
    }

    // Class, holding list of albums in user profile
    class user_album_list
    {
        static cfg_objList<audio_album_info> m_cfg_album_list;

        int find_album (const pfc::string_base & p_name) const
        {
            auto n = m_cfg_album_list.get_size ();
            while (n --> 0) {
                if (pfc::stringCompareCaseInsensitive (m_cfg_album_list[n].get<0> ().get_ptr (), p_name.get_ptr ()) == 0)
                    return n;
            }
            return -1;
        }

        void reset_error ()
        {
            m_aborted = false;
            m_error.reset ();
        }
            
        void get_method_error_code (const api_method_base & p_method)
        {
            m_aborted = p_method.aborted ();
            m_error = p_method.get_error ();
        }

        template <class t_api_method, typename t_on_ok_callback>
        bool call_api (t_api_method & p_method, const t_on_ok_callback & on_ok)
        {
            reset_error ();
            bool is_ok = p_method.call ();
            if (is_ok) on_ok (p_method);
            else get_method_error_code (p_method);
            return is_ok;
        }

        bool m_aborted;
        pfc::string8 m_error;

    public:
        const pfc::list_base_const_t<audio_album_info> & get_albums () const { return m_cfg_album_list; }

        bool reload (); // Loads album list from user vk profile

        bool add_item (const pfc::string_base & p_title); // Adds new album to user profile
        bool remove_item (t_vk_album_id album_id); // Deletes album from user profile
        bool rename_item (const pfc::string_base & p_current_name, const pfc::string_base & p_new_name);

        bool aborted () const { return m_aborted; }
        const char* get_error () const { return m_error.get_ptr (); }

        // Return 0 if not found
        t_vk_album_id get_album_id_by_name (const pfc::string_base & p_album_name) const
        {
            int item_index = find_album (p_album_name);
            return item_index > -1 ? m_cfg_album_list[item_index].get<1> () : 0;
        }

        // Does nothing with user profile
        void clear () { m_cfg_album_list.remove_all (); }
    };
}
#endif