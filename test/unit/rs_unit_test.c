
#include <rs_config.h>
#include <rs_core.h>
#include <CUnit/Basic.h>


/* static int rs_get_options(int argc, char *const *argv); */
static int rs_init_suite(void);
static int rs_clean_suite(void);


static void rs_ring_buffer2_test(void);
static void rs_slab_test(void);
static void rs_conf_test(void);


/* static rs_core_info_t  *ci; */
int main(int argc, char *const *argv)
{
    CU_pSuite pSuite = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    pSuite = CU_add_suite("rs_unit_test", rs_init_suite, rs_clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
    if ((NULL == CU_add_test(pSuite, "test of rs_slab_test", 
                    rs_slab_test)) || 
            (NULL == CU_add_test(pSuite, "test of rs_ring_buffer2_test", 
                                 rs_ring_buffer2_test)) ||
            (NULL == CU_add_test(pSuite, "test of rs_conf_test",
                                 rs_conf_test))
       )
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}

static int rs_init_suite(void)
{
    /* init rs_errno */
    if(rs_strerror_init() != RS_OK) {
        return 1;
    }

#if 0
    /* init core info */
    if((ci = rs_init_core_info(NULL)) == NULL) {
        return 1;
    }

    rs_core_info = ci;
#endif

    return 0;
}

static int rs_clean_suite(void)
{
    return 0;
}

static void rs_conf_test(void)
{
    rs_conf_t   conf;
    char        *conf_path;
    uint32_t    test, test2;
    char        *test1;

    conf_path = "etc/test.cf";
    test1= NULL;
    test = 0;
    test2= 0;

    rs_conf_t_init(&conf);

    CU_ASSERT(rs_add_conf_kv(&conf, "test", &test, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test1", &test1, RS_CONF_STR) == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test2", &test2, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test3", &test2, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test4", &test2, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test5", &test2, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test6", &test2, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test7", &test2, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test8", &test2, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test9", &test2, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test10", &test2, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test11", &test2, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test12", &test2, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test13", &test2, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test14", &test2, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test15", &test2, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test16", &test2, RS_CONF_UINT32) 
            == RS_OK);
    CU_ASSERT(rs_add_conf_kv(&conf, "test16", &test2, RS_CONF_UINT32) 
            == RS_ERR);

    CU_ASSERT(rs_init_conf(&conf, conf_path, RS_CORE_MODULE_NAME) == RS_OK);
    CU_ASSERT(test == 1);
    CU_ASSERT(rs_strcmp("abc", test1) == 0);
    CU_ASSERT(test2 == 1234);
}

static void rs_slab_test(void)
{
    return;
    rs_slab_t   slab;
    int         id, i;
    char        *p;

    /* init slab  */
    CU_ASSERT(rs_init_slab(&slab, NULL, 100, 1.5, 1024 * 1024 * 2, 
                RS_SLAB_PREALLOC) == RS_OK) 

        /* alloc slab */
        CU_ASSERT((id = rs_slab_clsid(&slab, 1747)) >= 0);

    for(i = 0; i < 599; i++) {
        CU_ASSERT(rs_alloc_slab(&slab, 1747, id) != NULL); 
    }

    CU_ASSERT((id = rs_slab_clsid(&slab, 100)) >= 0);

    for(i = 0; i < 10485; i++) {
        CU_ASSERT(rs_alloc_slab(&slab, 100, id) != NULL); 
    }

    /* no more mem */
    CU_ASSERT(rs_alloc_slab(&slab, 1747, id) == NULL); 

    CU_ASSERT((id = rs_slab_clsid(&slab, 1747)) >= 0);
    p = slab.slab_class[id].slabs[0];

    /* free slab */
    for(i = 0; i < 599; i++) {
        rs_free_slab(&slab, p, id);
        p = (char *) p + 1747;
    }

    CU_ASSERT((id = rs_slab_clsid(&slab, 100)) >= 0);
    p = slab.slab_class[id].slabs[1];

    /* free slab */
    for(i = 0; i < 10485; i++) {
        rs_free_slab(&slab, p, id);
        p = (char *) p + 100;
    }

    /* alloc slab */
    CU_ASSERT((id = rs_slab_clsid(&slab, 1747)) >= 0);

    for(i = 0; i < 599; i++) {
        CU_ASSERT(rs_alloc_slab(&slab, 1747, id) != NULL); 
    }

    CU_ASSERT((id = rs_slab_clsid(&slab, 100)) >= 0);

    for(i = 0; i < 10485; i++) {
        CU_ASSERT(rs_alloc_slab(&slab, 100, id) != NULL); 
    }
}

static void rs_ring_buffer2_test(void)
{
    return;
    int                     i;
    rs_ring_buffer2_t       rb; 
    rs_ring_buffer2_data_t  *d;

    /* init ring buffer2 */
    CU_ASSERT(rs_init_ring_buffer2(&rb, 10) == RS_OK);

    /* get ring buffer2 empty */
    CU_ASSERT(rs_get_ring_buffer2(&rb, &d) == RS_EMPTY);

    /* set ring buffer2 */
    for(i = 0; i < 10; i++) {
        CU_ASSERT(rs_set_ring_buffer2(&rb, &d) == RS_OK);
        rs_set_ring_buffer2_advance(&rb);
    }

    /* set ring buffer2 full */
    CU_ASSERT(rs_set_ring_buffer2(&rb, &d) == RS_FULL);

    /* get ring buffer2 */
    for(i = 0; i < 10; i++) {
        CU_ASSERT(rs_get_ring_buffer2(&rb, &d) == RS_OK);
        rs_get_ring_buffer2_advance(&rb);
    }

    CU_ASSERT(rs_get_ring_buffer2(&rb, &d) == RS_EMPTY);

}

#if 0
static int rs_get_options(int argc, char *const *argv) 
{
    char   *p;
    int     i;

    for (i = 1; i < argc; i++) {

        p = argv[i];

        if (*p++ != '-') {
            rs_log_stderr(0, "invalid option: \"%s\"", argv[i]);
            return RS_ERR;
        }

        while (*p) {

            switch (*p++) {
                case 'c':
                    if (*p) {
                        rs_conf_path = p;
                        goto next;
                    }

                    if (argv[++i]) {
                        rs_conf_path = argv[i];
                        goto next;
                    }

                    rs_log_stderr(0, "option \"-c\" requires config file");
                    return RS_ERR;

                default:
                    rs_log_stderr(0, "invalid option: \"%c\"", *(p - 1));
                    return RS_ERR;
            }
        }

next:

        continue;
    }

    return RS_OK;
}
#endif
