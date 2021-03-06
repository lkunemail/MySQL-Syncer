
#include <rs_config.h>
#include <rs_core.h>
#include <rs_master.h>

static int rs_filter_test(rs_binlog_info_t *bi);
void rs_create_test_event(Test *pb_t, void *data);

int rs_def_filter_data_handle(rs_request_dump_t *rd)
{
    rs_binlog_info_t    *bi;

    bi = &(rd->binlog_info);

    if(rs_strncmp(bi->db, RS_FILTER_DB_NAME, bi->dbl) != 0) {
        return RS_OK;
    }

    if(bi->data == NULL) {
        
        bi->data = malloc(RS_SYNC_DATA_SIZE);

        if(bi->data == NULL) {
            rs_log_err(rs_errno, "malloc() failed, rs_sync_data_size", 
                    RS_SYNC_DATA_SIZE);
            return RS_ERR;
        }
    }

    if(bi->tran == 1 && rs_memcmp(bi->sql, TEST_FILTER_KEY, 
                TEST_FILTER_KEY_LEN) == 0) {

        rs_mysql_test_t *t = (rs_mysql_test_t *) bi->data; 
        rs_mysql_test_t_init(t);

        /* INSERT TEST */
        if(rs_filter_test(bi) != RS_OK) {
            return RS_ERR;
        }

        bi->flush = 1;
        bi->mev = RS_MYSQL_INSERT_TEST;
    }

    return RS_OK;
}

int rs_def_create_data_handle(rs_request_dump_t *rd) 
{
    int                     i, r, len, id, pb_len;
    char                    *p, istr[UINT32_LEN];
    void                    *pb_buf;
    rs_binlog_info_t        *bi;
    rs_ring_buffer2_data_t  *d;
    rs_slab_t               *sl;

    i = 0;
    bi = &(rd->binlog_info);
    sl = &(rd->slab);

    for( ;; ) {

        r = rs_set_ring_buffer2(&(rd->ring_buf), &d);

        if(r == RS_FULL) {
            if(i % 60 == 0) {
                rs_log_info("request dump's ring buffer is full, wait %u "
                        "secs" ,RS_RING_BUFFER_FULL_SLEEP_SEC);
                i = 0;
            }

            i += RS_RING_BUFFER_FULL_SLEEP_SEC;

            sleep(RS_RING_BUFFER_FULL_SLEEP_SEC);

            continue;
        }

        if(r == RS_ERR) {
            return RS_ERR;
        }

        if(r == RS_OK) {

            /* free slab chunk */
            if(d->data != NULL && d->id > 0 && d->len > 0) {
                rs_free_slab_chunk(sl, d->data, d->id); 
            }

            rs_uint32_to_str(rd->dump_pos, istr);
            len = rs_strlen(rd->dump_file) + rs_strlen(istr) + 1 + 1 + 1; 

            if(bi->mev == 0) {

                d->len = len;
                d->id = rs_slab_clsid(sl, len);
                d->data = rs_alloc_slab(sl, len, d->id);

                if(d->data == NULL) {
                    return RS_ERR;
                }

                len = snprintf(d->data, d->len, "%s,%u,%c", rd->dump_file, 
                        rd->dump_pos, bi->mev);

                if(len < 0) {
                    rs_log_err(rs_errno, "snprint() failed, dump cmd"); 
                    return RS_ERR;
                }

            } else {
                Test t = TEST__INIT;
                rs_create_test_event(&t, bi->data);
                pb_len = test__get_packed_size(&t);

                /* alloc mem for pb */
                id = rs_slab_clsid(sl, pb_len);
                pb_buf = rs_alloc_slab(sl, pb_len, id);

                if(pb_buf == NULL) {
                    return RS_ERR;
                }

                len += pb_len;

                /* alloc mem for cmd */
                d->len = len;
                d->id = rs_slab_clsid(sl, len);
                d->data = rs_alloc_slab(sl, len, d->id);

                if(d->data == NULL) {
                    return RS_ERR;
                }

                len = snprintf(d->data, d->len, "%s,%u,%c", rd->dump_file, 
                        rd->dump_pos, bi->mev);

                if(len < 0) {
                    rs_log_err(rs_errno, "snprint() failed, dump cmd"); 
                    return RS_ERR;
                }

                p = (char *) d->data + len;
                test__pack(&t, pb_buf);
                rs_memcpy(p, pb_buf, pb_len);

                /* free pb buffer */
                rs_free_slab_chunk(sl, pb_buf, id);
            }

            rs_set_ring_buffer2_advance(&(rd->ring_buf));

            break;
        }

    } // EXIT RING BUFFER 

    bi->flush = 0;
    bi->mev = 0;
    bi->sent = 1;

    return RS_OK;
}

static int rs_filter_test(rs_binlog_info_t *bi)
{
    char            *p;
    rs_mysql_test_t *t;

    p = bi->sql;
    t = (rs_mysql_test_t *) bi->data;

    if(bi->auto_incr == 0) {
        rs_log_err(0, "filter test failed, no auto_incr");
    }

    /* COLUMN : ID */
    t->id = (uint32_t) bi->ai;

    if((p = rs_strstr_end(p, TEST_COLUMN_MSG, TEST_COLUMN_MSG_LEN)) == NULL) {
        rs_log_err(0, "filter test failed, \"%s\"", TEST_COLUMN_MSG);
        return RS_ERR;
    }

    /* COLUMN : MSG */
    p = rs_cp_utf8_str(t->msg, p);

    return RS_OK;
}

void rs_create_test_event(Test *pb_t, void *data)
{
    rs_mysql_test_t *t;

    t = (rs_mysql_test_t *) data;

    rs_log_debug(0, 
            "\n------------------------\n"
            "id             : %u\n" 
            "msg            : %s"
            "\n------------------------\n",
            t->id,
            t->msg
            );

    pb_t->id = t->id;
    pb_t->msg = t->msg;
}
