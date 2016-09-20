#include <curl/curl.h>

char* download_file(char* url, char* artist)
{
    CURL *curl;
    FILE *fp;
    CURLcode res;
    char *out, *images_dir, *ext;

    curl = curl_easy_init();
    if (curl)
    {
        sprintf(outdir, "%s", images_dir);
        sprintf(outfn, "%s%s", outdir, strrchr(url, '/' ));
        mkdir(outdir, 0777);
        if (access(outfn, F_OK) != -1) {
            syslog(LOG_INFO, "%s: file already exists %s\n", __func__, outfn);
            goto done;
        }
        fp = fopen(outfn,"wb");
        if (fp) {
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            res = curl_easy_perform(curl);
            fclose(fp);
            if (res != 0) {
                syslog(LOG_DEBUG, "%s error!\n", __func__);
                outdir = NULL;
                goto done;
            }
        }
    }
done:
    if (outdir)
        sprintf(outdir, "%s", strrchr(outfn, '/')+1);
    syslog(LOG_DEBUG, "%s downloaded %s\n", __func__, outdir);
    curl_easy_cleanup(curl);

    return outdir;
}

