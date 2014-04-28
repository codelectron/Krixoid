// vim: set foldmethod=syntax :
#include <errno.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <math.h>
extern "C" {
#include <oauth.h>
 }
#include <oauth.h>
#include <curl/curl.h>

#include "tweet.h"
#include "utils.h"

static union KEYS *keys = NULL;

union KEYS *register_keys(union KEYS *k) {
	return keys = k;
}

int check_keys(void) {
	return keys->keys_array[0]&&keys->keys_array[1]&&keys->keys_array[2]&&keys->keys_array[3];
}

int bear_init(union KEYS *k) {
	register_keys(k);
	return curl_global_init(CURL_GLOBAL_DEFAULT);
}

int bear_cleanup(void) {
	curl_global_cleanup();
	return 0;
}

static size_t write_data(char *buffer, size_t size, size_t nmemb, void *rep) {
	*(buffer + size * nmemb) = '\0';
		alloc_strcat((char**)rep, buffer);

	return size * nmemb;
}

static int http_request(char const *u, int p, char **rep) {
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (rep && *rep) {
		memset(*rep,0,strlen(*rep));
	}
	CURL *curl;
	CURLcode ret;
	curl = curl_easy_init();
	if (!curl) {
		fprintf(stderr, "failed to initialize curl\n");
	}
	char *request = NULL;
	char *post = NULL;
	if (p) {
		request = oauth_sign_url2(u, &post, OA_HMAC, NULL, keys->keys_struct.c_key, keys->keys_struct.c_sec, keys->keys_struct.t_key, keys->keys_struct.t_sec);
		curl_easy_setopt (curl, CURLOPT_POSTFIELDS, (void *) post);
	} else {
		request = oauth_sign_url2(u, NULL, OA_HMAC, NULL, keys->keys_struct.c_key, keys->keys_struct.c_sec, keys->keys_struct.t_key, keys->keys_struct.t_sec);
	}
	curl_easy_setopt (curl, CURLOPT_URL, request);
	//is it good? i dont know.
	curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0L);
	if (rep ) {
		curl_easy_setopt (curl, CURLOPT_WRITEDATA, (void *) rep);
		curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, write_data);
	}

	ret = curl_easy_perform (curl);
	if (ret != CURLE_OK) {
		fprintf (stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror (ret));
	}
	free(request);request = NULL;
	free(post);post = NULL;
	curl_easy_cleanup(curl);

	return ret;
}

char const *api_uri_1_1 = "https://api.twitter.com/1.1/";

char const * api_uri[] = {
	[STATUSES_MENTIONS_TIMELINE] = "statuses/mentions_timeline.json",
	[STATUSES_USER_TIMELINE] = "statuses/user_timeline.json",
	[STATUSES_HOME_TIMELINE]  = "statuses/home_timeline.json",
	[STATUSES_RETWEETS_OF_ME] = "statuses/retweets_of_me.json",
	[STATUSES_RETWEETS_BY_ID] = "statuses/retweets/",
	[STATUSES_SHOW_BY_ID] = "statuses/show.json",
	[STATUSES_DESTROY_BY_ID] = "statuses/destroy/",
	[STATUSES_UPDATE] = "statuses/update.json",
	[STATUSES_RETWEET_BY_ID] = "statuses/retweet/",
	[STATUSES_UPDATE_WITH_MEDIA] = "statuses/update_with_media.json",
	[STATUSES_OEMBED] = "statuses/oembed.json",
	[STATUSES_RETWEETERS_IDS] = "statuses/retweeters/ids.json",
	[SEARCH_TWEETS] = "search/tweets.json",
	[DIRECT_MESSAGES] = "direct_messages.json",
	[DM_SENT] = "direct_messages/sent.json",
	[DM_SHOW] = "direct_messages/show.json",
	[DM_DESTROY] = "direct_messages/destroy.json ",
	[DM_NEW] = "direct_messages/new.json",
	[FS_NO_RETWEETS_IDS] = "friendships/no_retweets/ids.json",
	[FRIENDS_IDS] = "friends/ids.json",
	[FOLLOWERS_IDS] = "followers/ids.json",
	[FS_LOOKUP] = "friendships/lookup.json",
	[FS_INCOMING] = "friendships/incoming.json",
	[FS_OUTGOING] = "friendships/outgoing.json",
	[FS_CREATE] = "friendships/create.json",
	[FS_DESTROY] = "friendships/destroy.json",
	[FS_UPDATE] = "friendships/update.json",
	[FS_SHOW] = "friendships/show.json",
	[FRIENDS_LIST] = "friends/list.json",
	[FOLLOWERS_LIST] = "followers/list.json",
	[ACCOUNT_SETTINGS] = "account/settings.json",
	[ACCOUNT_VERIFY_CREDEBTIALS] = "account/verify_credentials.json",
	[ACCOUNT_UPDATE_DELIVERY_DEVICE] = "account/update_delivery_device.json",
	[ACCOUNT_UPDATE_PROFILE] = "account/update_profile.json",
	[ACCOUNT_UPDATE_PROFILE_BACKGROUND_IMAGE] = "account/update_profile_background_image.json",
	[ACCOUNT_UPDATE_PROFILE_COLORS] = "account/update_profile_colors.json",
	[ACCOUNT_UPDATE_PROFILE_IMAGE] = "account/update_profile_image.json",
	[BLOCKS_LIST] = "blocks/list.json",
	[BLOCKS_IDS] = "blocks/ids.json",
	[BLOCKS_CREATE] = "blocks/create.json",
	[BLOCKS_DESTROY] = "blocks/destroy.json",
	[USERS_LOOKUP] = "users/lookup.json",
	[USERS_SHOW] = "users/show.json",
	[USERS_SEARCH] = "users/search.json",
	[USERS_CONTRIBUTEES] = "users/contributees.json",
	[USERS_CONTRIBUTORS] = "users/contributors.json",
	[ACCOUNT_REMOVE_PROFILE_BANNER] = "account/remove_profile_banner.json",
	[ACCOUNT_UPDATE_PROFILE_BANNER] = "account/update_profile_banner.json",
	[USERS_PROFILE_BANNER] = "users/profile_banner.json",
};

inline static char **add_que_or_amp(enum APIS api, char **uri) {
	alloc_strcat(uri, strlen(*uri)==(strlen(api_uri_1_1) + strlen(api_uri[api]))?"?":"&");
	return uri;
}

static char **add_count(enum APIS api, char **uri, int count) {
	if (!(count)) {
		return uri;
	}
	char cnt[8] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "count=");
	snprintf(cnt, sizeof(cnt), "%d", count<201?count:200);
	alloc_strcat(uri, cnt);

	return uri;
}

static char **add_id(enum APIS api, char **uri, tweet_id_t id) {
	if (!(id)) {
		return uri;
	}
	char i[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "id=");
	snprintf(i, sizeof(i), "%llu", id);
	alloc_strcat(uri, i);

	return uri;
}

static char **add_since_id(enum APIS api, char **uri, tweet_id_t since_id) {
	if (!(since_id)) {
		return uri;
	}
	char id[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "since_id=");
	snprintf(id, sizeof(id), "%llu", since_id);
	alloc_strcat(uri, id);

	return uri;
}

static char **add_max_id(enum APIS api, char **uri, tweet_id_t max_id) {
	if (!(max_id)) {
		return uri;
	}
	char id[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "max_id=");
	snprintf(id, sizeof(id), "%llu", max_id);
	alloc_strcat(uri, id);

	return uri;
}

static char **add_trim_user(enum APIS api, char **uri, int trim_user) {
	if (!(trim_user != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "trim_user=");
	snprintf(boolean, sizeof(boolean), "%d", !!trim_user);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_contributor_details(enum APIS api, char **uri, int contributor_details) {
	if (!(contributor_details != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "contributor_details=");
	snprintf(boolean, sizeof(boolean), "%d", !!contributor_details);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_include_entities(enum APIS api, char **uri, int include_entities) {
	if (!(include_entities != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "include_entities=");
	snprintf(boolean, sizeof(boolean), "%d", !!include_entities);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_include_rts(enum APIS api, char **uri, int include_rts, int count) {
	if (!(count || (include_rts != -1))) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "include_rts=");
	snprintf(boolean, sizeof(boolean), "%d", count || (include_rts != -1));
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_user_id(enum APIS api, char **uri, tweet_id_t user_id) {
	if (!(user_id)) {
		return uri;
	}
	char id[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "user_id=");
	snprintf(id, sizeof(id), "%llu", user_id);
	alloc_strcat(uri, id);

	return uri;
}

static char **add_screen_name(enum APIS api, char **uri, char *screen_name) {
	if (!(screen_name && *screen_name)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "screen_name=");
	alloc_strcat(uri, screen_name);

	return uri;
}

static char **add_exclude_replies(enum APIS api, char **uri, int exclude_replies) {
	if (!(exclude_replies != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "exclude_replies=");
	snprintf(boolean, sizeof(boolean), "%d", !!exclude_replies);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_include_user_entities(enum APIS api, char **uri, int include_user_entities) {
	if (!(include_user_entities != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "include_user_entities=");
	snprintf(boolean, sizeof(boolean), "%d", !!include_user_entities);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_include_my_retweet(enum APIS api, char **uri, int include_my_retweet) {
	if (!(include_my_retweet != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "include_my_retweet=");
	snprintf(boolean, sizeof(boolean), "%d", !!include_my_retweet);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_status(enum APIS api, char **uri, char *status) {
	if (!(status && *status)) {
		return uri;
	}
	char *status_140 = NULL;
	alloc_strcat(&status_140, status);
	if (utf8_strlen(status_140) > 140) {
		*(utf8_offset_to_pointer(status_140, 140)) = '\0';
	}
	char *escaped_msg = oauth_url_escape(status_140);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "status=");
	alloc_strcat(uri, escaped_msg);
	free(status_140);status_140 = NULL;
	free(escaped_msg);escaped_msg = NULL;

	return uri;
}

static char **add_in_reply_to_status_id(enum APIS api, char **uri, tweet_id_t in_reply_to_status_id) {
	if (!(in_reply_to_status_id)) {
		return uri;
	}
	char id[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "in_reply_to_status_id=");
	snprintf(id, sizeof(id), "%llu", in_reply_to_status_id);
	alloc_strcat(uri, id);

	return uri;
}

static char **add_coods(enum APIS api, char **uri, struct GEOCODE l_l) {
	if (!((int)(fabs(l_l.latitude)) < 90 && (int)(fabs(l_l.longitude)) < 180)) {
		return uri;
	}
	char latitude[32];
	char longitude[32];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "latitude=");
	snprintf(latitude, sizeof(latitude), "%2.12f", l_l.latitude);
	alloc_strcat(uri, latitude);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "longitude=");
	snprintf(longitude, sizeof(longitude), "%2.12f", l_l.longitude);
	alloc_strcat(uri, longitude);

	return uri;
}

static char **add_place_id(enum APIS api, char **uri, tweet_id_t place_id) {
	if (!(place_id)) {
		return uri;
	}
	char id[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "place_id=");
	snprintf(id, sizeof(id), "%llx", place_id);
	alloc_strcat(uri, id);

	return uri;
}

static char **add_display_coordinates(enum APIS api, char **uri, int display_coordinates) {
	if (!(display_coordinates != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "display_coordinates=");
	snprintf(boolean, sizeof(boolean), "%d", !!display_coordinates);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_url(enum APIS api, char **uri, char *url) {
	if (!(url && *url)) {
		return uri;
	}
	char *escaped_msg = oauth_url_escape(url);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "url=");
	alloc_strcat(uri, escaped_msg);
	free(escaped_msg);escaped_msg = NULL;

	return uri;
}

static char **add_maxwidth(enum APIS api, char **uri, int maxwidth) {
	if (!(249 < maxwidth && maxwidth < 551)) {
		return uri;
	}
	char cnt[8] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "maxwidth=");
	snprintf(cnt, sizeof(cnt), "%d", maxwidth);
	alloc_strcat(uri, cnt);

	return uri;
}

static char **add_hide_media(enum APIS api, char **uri, int hide_media) {
	if (!(hide_media != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "hide_media=");
	snprintf(boolean, sizeof(boolean), "%d", !!hide_media);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_hide_thread(enum APIS api, char **uri, int hide_thread) {
	if (!(hide_thread != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "hide_thread=");
	snprintf(boolean, sizeof(boolean), "%d", !!hide_thread);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_omit_script(enum APIS api, char **uri, int omit_script) {
	if (!(omit_script != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "omit_script=");
	snprintf(boolean, sizeof(boolean), "%d", !!omit_script);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_align(enum APIS api, char **uri, enum ALIGN align) {
	if (!(align < (CENTER + 1))) {
		return uri;
	}
	char const *algn[] = {"none", "left", "right", "center"};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "align=");
	alloc_strcat(uri, algn[align]);

	return uri;
}

static char **add_related(enum APIS api, char **uri, char *related) {
	if (!(related && *related)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "related=");
	alloc_strcat(uri, related);

	return uri;
}

static char **add_lang(enum APIS api, char **uri, char *lang) {
	if (!(lang && *lang)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "lang=");
	alloc_strcat(uri, lang);

	return uri;
}

static char **add_cursor(enum APIS api, char **uri, cursor_t cursor) {
	if (!(cursor)) {
		return uri;
	}
	char cur[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "cursor=");
	snprintf(cur, sizeof(cur), "%lld", cursor);
	alloc_strcat(uri, cur);

	return uri;
}

static char **add_stringify_ids(enum APIS api, char **uri, int stringify_ids) {
	if (!(stringify_ids != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "stringify_ids=");
	snprintf(boolean, sizeof(boolean), "%d", !!stringify_ids);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_q(enum APIS api, char **uri, char *q){
	if(!(q && *q)) {
		return uri;
	}
	char *escaped_msg = oauth_url_escape(q);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "q=");
	alloc_strcat(uri, escaped_msg);
	free(escaped_msg);escaped_msg = NULL;

	return uri;
}

static char **add_geocode(enum APIS api, char **uri, struct GEOCODE geocode) {
	if (!((int)(fabs(geocode.latitude)) < 90 && (int)(fabs(geocode.longitude)) < 180 && geocode.radius != 0 && geocode.unit && *geocode.unit)) {
		return uri;
	}
	char latitude[32];
	char longitude[32];
	char rad[8];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "geocode=");
	snprintf(latitude, sizeof(latitude), "%2.12f,", geocode.latitude);
	alloc_strcat(uri, latitude);
	snprintf(longitude, sizeof(longitude), "%2.12f,", geocode.longitude);
	alloc_strcat(uri, longitude);
	snprintf(rad, sizeof(rad), "%d", geocode.radius);
	alloc_strcat(uri, rad);
	alloc_strcat(uri, geocode.unit);

	return uri;
}

static char **add_locale(enum APIS api, char **uri, char *locale) {
	if (!(locale && *locale)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "locale=");
	alloc_strcat(uri, locale);

	return uri;
}

static char **add_result_type(enum APIS api, char **uri, int result_type) {
	if (!(result_type)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "result_type=");
	if (result_type & MIXED) {
		alloc_strcat(uri, "mixed");
	}
	if (result_type & RECENT) {
		if (result_type & MIXED) {
			alloc_strcat(uri, ",");
		}
		alloc_strcat(uri, "recent");
	}
	if (result_type & POPULAR) {
		if (result_type & (MIXED | RECENT)) {
			alloc_strcat(uri, ",");
		}
		alloc_strcat(uri, "popular");
	}

	return uri;
}

static char **add_until(enum APIS api, char **uri, char *until) {
	if (!(until && *until)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "until=");
	alloc_strcat(uri, until);

	return uri;
}

static char **add_callback(enum APIS api, char **uri, char *callback) {
	if (!(callback && *callback)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "callback=");
	alloc_strcat(uri, callback);

	return uri;
}

static char **add_skip_status(enum APIS api, char **uri, int skip_status) {
	if (!(skip_status != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "skip_status=");
	snprintf(boolean, sizeof(boolean), "%d", !!skip_status);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_pages(enum APIS api, char **uri, int pages) {
	if (!(pages)) {
		return uri;
	}
	char pg[8] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "pages=");
	snprintf(pg, sizeof(pg), "%d", pages);
	alloc_strcat(uri, pg);

	return uri;
}

static char **add_text(enum APIS api, char **uri, char *text) {
	if (!(text && *text)) {
		return uri;
	}
	char *escaped_msg = oauth_url_escape(text);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "text=");
	alloc_strcat(uri, escaped_msg);
	free(escaped_msg);escaped_msg = NULL;

	return uri;
}

static char **add_count_upto_5000(enum APIS api, char **uri, int count) {
	if (!(count)) {
		return uri;
	}
	char cnt[8] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "count=");
	snprintf(cnt, sizeof(cnt), "%d", count<5001?count:5000);
	alloc_strcat(uri, cnt);

	return uri;
}

static char **add_user_id_str(enum APIS api, char **uri, char *user_id) {
	if (!(user_id && *user_id)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "user_id=");
	alloc_strcat(uri, user_id);

	return uri;
}

static char **add_follow(enum APIS api, char **uri, int follow) {
	if (!(follow != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "follow=");
	snprintf(boolean, sizeof(boolean), "%d", !!follow);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_device(enum APIS api, char **uri, int device) {
	if (!(device != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "device=");
	snprintf(boolean, sizeof(boolean), "%d", !!device);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_retweets(enum APIS api, char **uri, int retweets) {
	if (!(retweets != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "retweets=");
	snprintf(boolean, sizeof(boolean), "%d", !!retweets);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_source_id(enum APIS api, char **uri, tweet_id_t source_id) {
	if (!(source_id)) {
		return uri;
	}
	char id[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "source_id=");
	snprintf(id, sizeof(id), "%llu", source_id);
	alloc_strcat(uri, id);

	return uri;
}

static char **add_source_screen_name(enum APIS api, char **uri, char *source_screen_name) {
	if (!(source_screen_name && *source_screen_name)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "source_screen_name=");
	alloc_strcat(uri, source_screen_name);

	return uri;
}

static char **add_target_id(enum APIS api, char **uri, tweet_id_t target_id) {
	if (!(target_id)) {
		return uri;
	}
	char id[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "target_id=");
	snprintf(id, sizeof(id), "%llu", target_id);
	alloc_strcat(uri, id);

	return uri;
}

static char **add_target_screen_name(enum APIS api, char **uri, char *target_screen_name) {
	if (!(target_screen_name && *target_screen_name)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "target_screen_name=");
	alloc_strcat(uri, target_screen_name);

	return uri;
}

static char **add_trend_location_woeid(enum APIS api, char **uri, int trend_location_woeid) {
	if (!(trend_location_woeid)) {
		return uri;
	}
	char woeid[12] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "trend_location_woeid=");
	snprintf(woeid, sizeof(woeid), "%d", trend_location_woeid);
	alloc_strcat(uri, woeid);

	return uri;
}

static char **add_sleep_time_enabled(enum APIS api, char **uri, int sleep_time_enabled) {
	if (!(sleep_time_enabled != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "sleep_time_enabled=");
	snprintf(boolean, sizeof(boolean), "%d", !!sleep_time_enabled);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_start_sleep_time(enum APIS api, char **uri, int start_sleep_time) {
	if (!(start_sleep_time)) {
		return uri;
	}
	char time[12] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "start_sleep_time=");
	snprintf(time, sizeof(time), "%d", start_sleep_time);
	alloc_strcat(uri, time);

	return uri;
}

static char **add_end_sleep_time(enum APIS api, char **uri, int end_sleep_time) {
	if (!(end_sleep_time)) {
		return uri;
	}
	char time[12] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "end_sleep_time=");
	snprintf(time, sizeof(time), "%d", end_sleep_time);
	alloc_strcat(uri, time);

	return uri;
}

static char **add_time_zone(enum APIS api, char **uri, char *time_zone) {
	if (!(time_zone && *time_zone)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "time_zone=");
	alloc_strcat(uri, time_zone);

	return uri;
}

static char **add_device_str(enum APIS api, char **uri, char *device) {
	if (!(device && *device)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "device=");
	alloc_strcat(uri, device);

	return uri;
}

static char **add_name(enum APIS api, char **uri, char *name) {
	if (!(name && *name)) {
		return uri;
	}
	char *name_20 = NULL;
	alloc_strcat(&name_20, name);
	if (utf8_strlen(name_20) > 20) {
		*(utf8_offset_to_pointer(name_20, 20)) = '\0';
	}
	char *escaped_msg = oauth_url_escape(name_20);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "name=");
	alloc_strcat(uri, escaped_msg);
	free(name_20);name_20 = NULL;
	free(escaped_msg);escaped_msg = NULL;

	return uri;
}

static char **add_url_upto_100(enum APIS api, char **uri, char *url){
	if (!(url && *url)) {
		return uri;
	}
	char *url_100 = NULL;
	alloc_strcat(&url_100, url);
	if (utf8_strlen(url_100) > 100) {
		*(utf8_offset_to_pointer(url_100, 100)) = '\0';
	}
	char *escaped_msg = oauth_url_escape(url_100);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "url=");
	alloc_strcat(uri, escaped_msg);
	free(url_100);url_100 = NULL;
	free(escaped_msg);escaped_msg = NULL;

	return uri;
}

static char **add_location(enum APIS api, char **uri, char *location){
	if (!(location && *location)) {
		return uri;
	}
	char *location_30 = NULL;
	alloc_strcat(&location_30, location);
	if (utf8_strlen(location_30) > 30) {
		*(utf8_offset_to_pointer(location_30, 30)) = '\0';
	}
	char *escaped_msg = oauth_url_escape(location_30);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "location=");
	alloc_strcat(uri, escaped_msg);
	free(location_30);location_30 = NULL;
	free(escaped_msg);escaped_msg = NULL;

	return uri;
}

static char **add_description(enum APIS api, char **uri, char *description){
	if (!(description && *description)) {
		return uri;
	}
	char *description_160 = NULL;
	alloc_strcat(&description_160, description);
	if (utf8_strlen(description_160) > 160) {
		*(utf8_offset_to_pointer(description_160, 160)) = '\0';
	}
	char *escaped_msg = oauth_url_escape(description_160);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "description=");
	alloc_strcat(uri, escaped_msg);
	free(description_160);description_160 = NULL;
	free(escaped_msg);escaped_msg = NULL;

	return uri;
}

static inline char **add_color(char **uri, int color, int digit){
	char hex[7] = {0};
	snprintf(hex, sizeof(hex), "%0*x", digit?digit:6, color);
	alloc_strcat(uri, hex);
	return uri;
}

static char **add_profile_background_color(enum APIS api, char **uri, long profile_background_color){
	if (!(profile_background_color > -1)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "profile_background_color=");
	add_color(uri, profile_background_color & 0x00ffffff, (profile_background_color & 0x0f000000) >> 24);

	return uri;
}

static char **add_profile_link_color(enum APIS api, char **uri, long profile_link_color){
	if (!(profile_link_color > -1)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "profile_link_color=");
	add_color(uri, profile_link_color & 0x00ffffff, (profile_link_color & 0x0f000000) >> 24);

	return uri;
}

static char **add_profile_sidebar_border_color(enum APIS api, char **uri, long profile_sidebar_border_color){
	if (!(profile_sidebar_border_color > -1)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "profile_sidebar_border_color=");
	add_color(uri, profile_sidebar_border_color & 0x00ffffff, (profile_sidebar_border_color & 0x0f000000) >> 24);

	return uri;
}

static char **add_profile_sidebar_fill_color(enum APIS api, char **uri, long profile_sidebar_fill_color){
	if (!(profile_sidebar_fill_color > -1)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "profile_sidebar_fill_color=");
	add_color(uri, profile_sidebar_fill_color & 0x00ffffff, (profile_sidebar_fill_color & 0x0f000000) >> 24);

	return uri;
}

static char **add_profile_text_color(enum APIS api, char **uri, long profile_text_color){
	if (!(profile_text_color > -1)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "profile_text_color=");
	add_color(uri, profile_text_color & 0x00ffffff, (profile_text_color & 0x0f000000) >> 24);

	return uri;
}

static char **add_page(enum APIS api, char **uri, int page) {
	if (!(page)) {
		return uri;
	}
	char pg[8] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "page=");
	snprintf(pg, sizeof(pg), "%d", page);
	alloc_strcat(uri, pg);

	return uri;
}

static char **add_count_upto_20(enum APIS api, char **uri, int count) {
	if (!(count)) {
		return uri;
	}
	char cnt[8] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "count=");
	snprintf(cnt, sizeof(cnt), "%d", count<21?count:20);
	alloc_strcat(uri, cnt);

	return uri;
}

int get_statuses_mentions_timeline (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	tweet_id_t since_id, //optional. if not 0, add it to argument.
	tweet_id_t max_id, //optional. if not 0, add it to argument.
	int trim_user, //optional. if not -1, add it to argument.
	int contributor_details, //optional. if not -1, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int include_rts //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/mentions_timeline.json
Parameters
count optional

Specifies the number of tweets to try and retrieve, up to a maximum of 200. The value of count is best thought of as a limit to the number of tweets to return because suspended or deleted content is removed after the count has been applied. We include retweets in the count, even if include_rts is not supplied. It is recommended you always send include_rts=1 when using this API method.

since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

contributor_details optional

This parameter enhances the contributors element of the status response to include the screen_name of the contributor. By default only the user_id of the contributor is included.

Example Values: true

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = STATUSES_MENTIONS_TIMELINE;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_count(api, &uri, count);
	add_since_id(api, &uri, since_id);
	add_max_id(api, &uri, max_id);
	add_trim_user(api, &uri, trim_user);
	add_contributor_details(api, &uri, contributor_details);
	add_include_entities(api, &uri, include_entities);
	add_include_rts(api, &uri, include_rts, count);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_statuses_user_timeline (
	char **res, //response
	tweet_id_t user_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	char *screen_name, //Always specify either an user_id or screen_name when requesting a user timeline.
	int count, //optional. if not 0, add it to argument.
	tweet_id_t since_id, //optional. if not 0, add it to argument.
	tweet_id_t max_id, //optional. if not 0, add it to argument.
	int trim_user, //optional. if not -1, add it to argument.
	int exclude_replies, //optional. if not -1, add it to argument.
	int contributor_details, //optional. if not -1, add it to argument.
	int include_rts //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/user_timeline.json
Parameters

Always specify either an user_id or screen_name when requesting a user timeline.

user_id optional

The ID of the user for whom to return results for.

Example Values: 12345

screen_name optional

The screen name of the user for whom to return results for.

Example Values: noradio

since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

count optional

Specifies the number of tweets to try and retrieve, up to a maximum of 200 per distinct request. The value of count is best thought of as a limit to the number of tweets to return because suspended or deleted content is removed after the count has been applied. We include retweets in the count, even if include_rts is not supplied. It is recommended you always send include_rts=1 when using this API method.

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

exclude_replies optional

This parameter will prevent replies from appearing in the returned timeline. Using exclude_replies with the count parameter will mean you will receive up-to count tweets — this is because the count parameter retrieves that many tweets before filtering out retweets and replies. This parameter is only supported for JSON and XML responses.

Example Values: true

contributor_details optional

This parameter enhances the contributors element of the status response to include the screen_name of the contributor. By default only the user_id of the contributor is included.

Example Values: true

include_rts optional

When set to false, the timeline will strip any native retweets (though they will still count toward both the maximal length of the timeline and the slice selected by the count parameter). Note: If you're using the trim_user parameter in conjunction with include_rts, the retweets will still contain a full user object.

Example Values: false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!(user_id || (screen_name && screen_name[0]))) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = STATUSES_USER_TIMELINE;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_user_id(api, &uri, user_id);
	add_screen_name(api, &uri, screen_name);
	add_count(api, &uri, count);
	add_since_id(api, &uri, since_id);
	add_max_id(api, &uri, max_id);
	add_trim_user(api, &uri, trim_user);
	add_exclude_replies(api, &uri, exclude_replies);
	add_contributor_details(api, &uri, contributor_details);
	add_include_rts(api, &uri, include_rts, count);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_statuses_home_timeline (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	tweet_id_t since_id, //optional. if not 0, add it to argument.
	tweet_id_t max_id, //optional. if not 0, add it to argument.
	int trim_user, //optional. if not -1, add it to argument.
	int exclude_replies, //optional. if not -1, add it to argument.
	int contributor_details, //optional. if not -1, add it to argument.
	int include_entities //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/home_timeline.json
Parameters
count optional

Specifies the number of records to retrieve. Must be less than or equal to 200. Defaults to 20.

Example Values: 5

since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

exclude_replies optional

This parameter will prevent replies from appearing in the returned timeline. Using exclude_replies with the count parameter will mean you will receive up-to count tweets — this is because the count parameter retrieves that many tweets before filtering out retweets and replies.

Example Values: true

contributor_details optional

This parameter enhances the contributors element of the status response to include the screen_name of the contributor. By default only the user_id of the contributor is included.

Example Values: true

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = STATUSES_HOME_TIMELINE;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_count(api, &uri, count);
	add_since_id(api, &uri, since_id);
	add_max_id(api, &uri, max_id);
	add_trim_user(api, &uri, trim_user);
	add_exclude_replies(api, &uri, exclude_replies);
	add_contributor_details(api, &uri, contributor_details);
	add_include_entities(api, &uri, include_entities);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_statuses_retweets_of_me (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	tweet_id_t since_id, //optional. if not 0, add it to argument.
	tweet_id_t max_id, //optional. if not 0, add it to argument.
	int trim_user, //optional. if not -1, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int include_user_entities //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/retweets_of_me.json
Parameters
count optional

Specifies the number of records to retrieve. Must be less than or equal to 100. If omitted, 20 will be assumed.

Example Values: 5

since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

include_entities optional

The tweet entities node will be disincluded when set to false.

Example Values: false

include_user_entities optional

The user entities node will be disincluded when set to false.

Example Values: false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = STATUSES_RETWEETS_OF_ME;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_count(api, &uri, count);
	add_since_id(api, &uri, since_id);
	add_max_id(api, &uri, max_id);
	add_trim_user(api, &uri, trim_user);
	add_include_entities(api, &uri, include_entities);
	add_include_user_entities(api, &uri, include_user_entities);


	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_statuses_retweets_by_id (
	tweet_id_t id, //required
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	int trim_user //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/retweets/:id.json
Parameters
id required

The numerical ID of the desired status.

Example Values: 123

count optional

Specifies the number of records to retrieve. Must be less than or equal to 100.

Example Values: 5

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!id) {
		fprintf(stderr, "need id number\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = STATUSES_RETWEETS_BY_ID;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);
	char i[32] = {0};
	snprintf(i, sizeof(i), "%lld.json", id);
	alloc_strcat(&uri, i);

	count%=101;
	add_count(api, &uri, count);
	add_trim_user(api, &uri, trim_user);


	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_statuses_show_by_id (
	tweet_id_t id, //required
	char **res, //response
	int trim_user, //optional. if not -1, add it to argument.
	int include_my_retweet, //optional. if not -1, add it to argument.
	int include_entities //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/show.json
Parameters
id required

The numerical ID of the desired Tweet.

Example Values: 123

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

include_my_retweet optional

When set to either true, t or 1, any Tweets returned that have been retweeted by the authenticating user will include an additional current_user_retweet node, containing the ID of the source status for the retweet.

Example Values: true

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!id) {
		fprintf(stderr, "need id number\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = STATUSES_SHOW_BY_ID;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_id(api, &uri, id);
	add_trim_user(api, &uri, trim_user);
	add_include_my_retweet(api, &uri, include_my_retweet);
	add_include_entities(api, &uri, include_entities);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int post_statuses_destroy_by_id (
	tweet_id_t id, //required
	char **res, //response
	int trim_user //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/destroy/:id.json
Parameters
id required

The numerical ID of the desired status.

Example Values: 123

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!id) {
		fprintf(stderr, "need id number\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = STATUSES_DESTROY_BY_ID;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);
	char i[32] = {0};
	snprintf(i, sizeof(i), "%lld.json", id);
	alloc_strcat(&uri, i);

	add_trim_user(api, &uri, trim_user);


	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

int post_statuses_update(
	char *status, //required
	char **res, // response
	tweet_id_t in_reply_to_status_id, //optional. if not 0, add it to argument.
	int do_add_l_l, //add it. whether add l_l to argument.
	struct GEOCODE l_l, //optional. if it is valid figure, add it to argument.
	tweet_id_t place_id, //optional. if not 0, add it to argument.
	int display_coordinates, //optional. if not -1, add it to argument.
	int trim_user //optional. if not -1, add it to argument.
	)
{
/*

Resource URL

https://api.twitter.com/1.1/statuses/update.json
Parameters
status required

The text of your status update, typically up to 140 characters. URL encode as necessary. t.co link wrapping may effect character counts.

There are some special commands in this field to be aware of. For instance, preceding a message with "D " or "M " and following it with a screen name can create a direct message to that user if the relationship allows for it.

in_reply_to_status_id optional

The ID of an existing status that the update is in reply to.

Note:: This parameter will be ignored unless the author of the tweet this parameter references is mentioned within the status text. Therefore, you must include @username, where username is the author of the referenced tweet, within the update.

lat optional

The latitude of the location this tweet refers to. This parameter will be ignored unless it is inside the range -90.0 to +90.0 (North is positive) inclusive. It will also be ignored if there isn't a corresponding long parameter.

Example Values: 37.7821120598956

long optional

The longitude of the location this tweet refers to. The valid ranges for longitude is -180.0 to +180.0 (East is positive) inclusive. This parameter will be ignored if outside that range, if it is not a number, if geo_enabled is disabled, or if there not a corresponding lat parameter.

Example Values: -122.400612831116

place_id optional

A place in the world. These IDs can be retrieved from GET geo/reverse_geocode.

Example Values: df51dec6f4ee2b2c

display_coordinates optional

Whether or not to put a pin on the exact coordinates a tweet has been sent from.

Example Values: true

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

*/

	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!status[0]) {
		fprintf(stderr, "need status text\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = STATUSES_UPDATE;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_status(api, &uri, status);
	add_in_reply_to_status_id(api, &uri, in_reply_to_status_id);
	if (do_add_l_l) {
		add_coods(api, &uri, l_l);
	}
	add_place_id(api, &uri, place_id);
	add_display_coordinates(api, &uri, display_coordinates);
	add_trim_user(api, &uri, trim_user);


	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

int post_statuses_retweet_by_id (
	tweet_id_t id, //required
	char **res, //response
	int trim_user //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/retweet/:id.json
Parameters
id required

The numerical ID of the desired status.

Example Values: 123

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!id) {
		fprintf(stderr, "need id number\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = STATUSES_RETWEET_BY_ID;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);
	char i[32] = {0};
	snprintf(i, sizeof(i), "%lld.json", id);
	alloc_strcat(&uri, i);

	add_trim_user(api, &uri, trim_user);


	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

//POST statuses/update_with_media is too difficult to implement

int get_statuses_oembed (
	tweet_id_t id, //required. It is not necessary to include both.
	char *url, //required. It is not necessary to include both.
	char **res, //response
	int maxwidth, //optional? It must be between 250 and 550.
	int hide_media, //optional? If not -1, add it to argument.
	int hide_thread, //optional? If not -1, add it to argument.
	int omit_script, //optional? If not -1, add it to argument.
	enum ALIGN align, //optional? If not NONE, add it to argument.
	char *related, //optional? If it is valid, add it to argument.
	char *lang //optional? If it is valid, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/oembed.json
Parameters

Either the id or url parameters must be specified in a request. It is not necessary to include both.
id required

The Tweet/status ID to return embed code for.

Example Values: 99530515043983360

url required

The URL of the Tweet/status to be embedded.

Example Values:

To embed the Tweet at https://twitter.com/#!/twitter/status/99530515043983360, use:

https%3A%2F%2Ftwitter.com%2F%23!%2Ftwitter%2Fstatus%2F99530515043983360

To embed the Tweet at https://twitter.com/twitter/status/99530515043983360, use:

https%3A%2F%2Ftwitter.com%2Ftwitter%2Fstatus%2F99530515043983360

maxwidth

The maximum width in pixels that the embed should be rendered at. This value is constrained to be between 250 and 550 pixels.

Note that Twitter does not support the oEmbed maxheight parameter. Tweets are fundamentally text, and are therefore of unpredictable height that cannot be scaled like an image or video. Relatedly, the oEmbed response will not provide a value for height. Implementations that need consistent heights for Tweets should refer to the hide_thread and hide_media parameters below.

Example Values: 325

hide_media

Specifies whether the embedded Tweet should automatically expand images which were uploaded via POST statuses/update_with_media. When set to either true, t or 1 images will not be expanded. Defaults to false.

Example Values: true

hide_thread

Specifies whether the embedded Tweet should automatically show the original message in the case that the embedded Tweet is a reply. When set to either true, t or 1 the original Tweet will not be shown. Defaults to false.

Example Values: true

omit_script

Specifies whether the embedded Tweet HTML should include a <script> element pointing to widgets.js. In cases where a page already includes widgets.js, setting this value to true will prevent a redundant script element from being included. When set to either true, t or 1 the <script> element will not be included in the embed HTML, meaning that pages must include a reference to widgets.js manually. Defaults to false.

Example Values: true

align

Specifies whether the embedded Tweet should be left aligned, right aligned, or centered in the page. Valid values are left, right, center, and none. Defaults to none, meaning no alignment styles are specified for the Tweet.

Example Values: center

related

A value for the TWT related parameter, as described in Web Intents. This value will be forwarded to all Web Intents calls.

Example Values:

twitterapi,twittermedia,twitter

lang

Language code for the rendered embed. This will affect the text and localization of the rendered HTML.

Example Values: fr

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!id || !url || !(*url)) {
		fprintf(stderr, "need id number or url text.\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = STATUSES_OEMBED;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_id(api, &uri, id);
	add_url(api, &uri, url);
	add_maxwidth(api, &uri, maxwidth);
	add_hide_media(api, &uri, hide_media);
	add_hide_thread(api, &uri, hide_thread);
	add_omit_script(api, &uri, omit_script);
	add_align(api, &uri, align);
	add_related(api, &uri, related);
	add_lang(api, &uri, lang);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_statuses_retweeters_ids (
	tweet_id_t id, //required
	char **res, //response
	cursor_t cursor, //optional. if not 0, add it to argument.
	int stringify_ids //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/retweeters/ids.json
Parameters
id required

The numerical ID of the desired status.

Example Values: 327473909412814850

cursor semi-optional

Causes the list of IDs to be broken into pages of no more than 100 IDs at a time. The number of IDs returned is not guaranteed to be 100 as suspended users are filtered out after connections are queried. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

While this method supports the cursor parameter, the entire result set can be returned in a single cursored collection. Using the count parameter with this method will not provide segmented cursors for use with this parameter.

Example Values: 12893764510938

stringify_ids optional

Many programming environments will not consume our ids due to their size. Provide this option to have ids returned as strings instead. Read more about Twitter IDs, JSON and Snowflake.

Example Values: true

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!id) {
		fprintf(stderr, "need id number\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = STATUSES_RETWEETERS_IDS;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_id(api, &uri, id);
	add_cursor(api, &uri, cursor);
	add_stringify_ids(api, &uri, stringify_ids);


	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_search_tweets (
	char *q, //required
	char **res, //response
	struct GEOCODE geocode, //optional. If it is valid, add it to argument.
	char *lang, //optional. If not 0, add it to argument.
	char *locale, //optional. If not 0, add it to argument. Only ja is currently effective
	int result_type, //optional. If not 0, add it to argument. 1 = "mixed",2="recent",4="popular"
	int count, //optional. If not 0, add it to argument.
	char *until, //optional. If not 0, add it to argument.
	tweet_id_t since_id, //optional. If not 0, add it to argument.
	tweet_id_t max_id, //optional. If not 0, add it to argument.
	int include_entities, //optional. If not -1, add it to argument.
	char *callback //optional. If not 0, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/search/tweets.json
Parameters
q required

A UTF-8, URL-encoded search query of 1,000 characters maximum, including operators. Queries may additionally be limited by complexity.

Example Values: @noradio

geocode optional

Returns tweets by users located within a given radius of the given latitude/longitude. The location is preferentially taking from the Geotagging API, but will fall back to their Twitter profile. The parameter value is specified by "latitude,longitude,radius", where radius units must be specified as either "mi" (miles) or "km" (kilometers). Note that you cannot use the near operator via the API to geocode arbitrary locations; however you can use this geocode parameter to search near geocodes directly. A maximum of 1,000 distinct "sub-regions" will be considered when using the radius modifier.

Example Values: 37.781157,-122.398720,1mi

lang optional

Restricts tweets to the given language, given by an ISO 639-1 code. Language detection is best-effort.

Example Values: eu

locale optional

Specify the language of the query you are sending (only ja is currently effective). This is intended for language-specific consumers and the default should work in the majority of cases.

Example Values: ja

result_type optional

Optional. Specifies what type of search results you would prefer to receive. The current default is "mixed." Valid values include:
  * mixed: Include both popular and real time results in the response.
  * recent: return only the most recent results in the response
  * popular: return only the most popular results in the response.

Example Values: mixed, recent, popular

count optional

The number of tweets to return per page, up to a maximum of 100. Defaults to 15. This was formerly the "rpp" parameter in the old Search API.

Example Values: 100

until optional

Returns tweets generated before the given date. Date should be formatted as YYYY-MM-DD. Keep in mind that the search index may not go back as far as the date you specify here.

Example Values: 2012-09-01

since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

callback optional

If supplied, the response will use the JSONP format with a callback of the given name. The usefulness of this parameter is somewhat diminished by the requirement of authentication for requests to this endpoint.

Example Values: processTweets

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!q || !(*q)) {
		fprintf(stderr, "need q text\n");
		return 0;
	}

	if (strlen(q) > 1000) {
		fprintf(stderr, "too long q text\n");
		return 0;
	}
	char *uri = NULL;
	enum APIS api = SEARCH_TWEETS;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_q(api, &uri, q);
	add_geocode(api, &uri, geocode);
	add_lang(api, &uri, lang);
	add_locale(api, &uri, locale);
	add_result_type(api, &uri, result_type);
	add_count(api, &uri, count);
	add_until(api, &uri, until);
	add_since_id(api, &uri, since_id);
	add_max_id(api, &uri, max_id);
	add_include_entities(api, &uri, include_entities);
	add_callback(api, &uri, callback);


	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_direct_messages (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	tweet_id_t since_id, //optional. if not 0, add it to argument.
	tweet_id_t max_id, //optional. if not 0, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	) {
/*

Resource URL
https://api.twitter.com/1.1/direct_messages.json
Parameters
since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

count optional

Specifies the number of direct messages to try and retrieve, up to a maximum of 200. The value of count is best thought of as a limit to the number of Tweets to return because suspended or deleted content is removed after the count has been applied.

Example Values: 5

include_entities optional

The entities node will not be included when set to false.

Example Values: false

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = DIRECT_MESSAGES;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_count(api, &uri, count);
	add_since_id(api, &uri, since_id);
	add_max_id(api, &uri, max_id);
	add_include_entities(api, &uri, include_entities);
	add_skip_status(api, &uri, skip_status);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_dm_sent (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	tweet_id_t since_id, //optional. if not 0, add it to argument.
	tweet_id_t max_id, //optional. if not 0, add it to argument.
	int pages, //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	int include_entities //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/direct_messages/sent.json
Parameters
since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

count optional

Specifies the number of records to retrieve. Must be less than or equal to 200.

Example Values: 5

page optional

Specifies the page of results to retrieve.

Example Values: 3

include_entities optional

The entities node will not be included when set to false.

Example Values: false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = DM_SENT;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_count(api, &uri, count);
	add_since_id(api, &uri, since_id);
	add_max_id(api, &uri, max_id);
	add_pages(api, &uri,pages);
	add_include_entities(api, &uri, include_entities);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_dm_show (
	tweet_id_t id, //required
	char **res //response
	) {
/*
Resource URL
https://api.twitter.com/1.1/direct_messages/show.json
Parameters
id required

The ID of the direct message.

Example Values: 587424932

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = DM_SHOW;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_id(api, &uri, id);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int post_dm_destroy (
	tweet_id_t id, //required
	char **res, //response
	int include_entities //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/direct_messages/destroy.json
Parameters
id required

The ID of the direct message to delete.

Example Values: 1270516771

include_entities optional

The entities node will not be included when set to false.

Example Values: false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = DM_DESTROY;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_id(api, &uri, id);
	add_include_entities(api, &uri, include_entities);

	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

int post_dm_new (
	tweet_id_t user_id, //One of user_id or screen_name are required.
	char *screen_name, //One of user_id or screen_name are required.
	char *text, //required.
	char **res //response
	) {
/*
Resource URL
https://api.twitter.com/1.1/direct_messages/new.json
Parameters

One of user_id or screen_name are required.

user_id optional

The ID of the user who should receive the direct message. Helpful for disambiguating when a valid user ID is also a valid screen name.

Example Values: 12345

screen_name optional

The screen name of the user who should receive the direct message. Helpful for disambiguating when a valid screen name is also a user ID.

Example Values: noradio
text required

The text of your direct message. Be sure to URL encode as necessary, and keep the message under 140 characters.

Example Values: Meet me behind the cafeteria after school

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!(user_id || (screen_name && screen_name[0]))) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = DM_NEW;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_user_id(api, &uri, user_id);
	add_screen_name(api, &uri, screen_name);
	add_text(api, &uri, text);

	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

int get_fs_no_retweets_ids (
	char **res, //response
	int stringify_ids //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/friendships/no_retweets/ids.json
Parameters
stringify_ids optional

Many programming environments will not consume our ids due to their size. Provide this option to have ids returned as strings instead. Read more about Twitter IDs, JSON and Snowflake. This parameter is especially important to use in Javascript environments.

Example Values: true

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = FS_NO_RETWEETS_IDS;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_stringify_ids(api, &uri, stringify_ids);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_friends_ids (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	cursor_t cursor, //optional. if not 0, add it to argument.
	int stringify_ids, //optional. if not -1, add it to argument.
	int count //optional. if not 0, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/friends/ids.json
Parameters

Either a screen_name or a user_id must be provided.

user_id optional

The ID of the user for whom to return results for.

Example Values: 12345

screen_name optional

The screen name of the user for whom to return results for.

Example Values: noradio

cursor semi-optional

Causes the list of connections to be broken into pages of no more than 5000 IDs at a time. The number of IDs returned is not guaranteed to be 5000 as suspended users are filtered out after connections are queried. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

stringify_ids optional

Many programming environments will not consume our Tweet ids due to their size. Provide this option to have ids returned as strings instead. More about Twitter IDs, JSON and Snowflake.

Example Values: true

count optional

Specifies the number of IDs attempt retrieval of, up to a maximum of 5,000 per distinct request. The value of count is best thought of as a limit to the number of results to return. When using the count parameter with this method, it is wise to use a consistent count value across all requests to the same user's collection. Usage of this parameter is encouraged in environments where all 5,000 IDs constitutes too large of a response.

Example Values: 2048

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!(user_id || (screen_name && screen_name[0]))) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = FRIENDS_IDS;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_user_id(api, &uri, user_id);
	add_screen_name(api, &uri, screen_name);
	add_cursor(api, &uri, cursor);
	add_stringify_ids(api, &uri, stringify_ids);
	add_count_upto_5000(api, &uri, count);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_followers_ids (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	cursor_t cursor, //optional. if not 0, add it to argument.
	int stringify_ids, //optional. if not -1, add it to argument.
	int count //optional. if not 0, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/followers/ids.json

Parameters

Either a screen_name or a user_id must be provided.

user_id optional

The ID of the user for whom to return results for.

Example Values: 12345

screen_name optional

The screen name of the user for whom to return results for.

Example Values: noradio

cursor semi-optional

Causes the list of connections to be broken into pages of no more than 5000 IDs at a time. The number of IDs returned is not guaranteed to be 5000 as suspended users are filtered out after connections are queried. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

stringify_ids optional

Many programming environments will not consume our Tweet ids due to their size. Provide this option to have ids returned as strings instead. More about Twitter IDs, JSON and Snowflake.

Example Values: true

count optional

Specifies the number of IDs attempt retrieval of, up to a maximum of 5,000 per distinct request. The value of count is best thought of as a limit to the number of results to return. When using the count parameter with this method, it is wise to use a consistent count value across all requests to the same user's collection. Usage of this parameter is encouraged in environments where all 5,000 IDs constitutes too large of a response.

Example Values: 2048

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!(user_id || (screen_name && screen_name[0]))) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = FOLLOWERS_IDS;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_user_id(api, &uri, user_id);
	add_screen_name(api, &uri, screen_name);
	add_cursor(api, &uri, cursor);
	add_stringify_ids(api, &uri, stringify_ids);
	add_count_upto_5000(api, &uri, count);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_fs_lookup (
	char **res, //response
	char *screen_name, //optional. if not 0, add it to argument.
	char *user_id //optional. if not 0, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/friendships/lookup.json
Parameters
screen_name optional

A comma separated list of screen names, up to 100 are allowed in a single request.

Example Values: twitterapi,twitter

user_id optional

A comma separated list of user IDs, up to 100 are allowed in a single request.

Example Values: 783214,6253282

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = FS_LOOKUP;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_screen_name(api, &uri, screen_name);
	add_user_id_str(api, &uri, user_id);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_fs_incoming (
	char **res, //response
	cursor_t cursor, //optional. if not 0, add it to argument.
	int stringify_ids //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/friendships/incoming.json
Parameters
cursor semi-optional

Causes the list of connections to be broken into pages of no more than 5000 IDs at a time. The number of IDs returned is not guaranteed to be 5000 as suspended users are filtered out after connections are queried. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

stringify_ids optional

Many programming environments will not consume our Tweet ids due to their size. Provide this option to have ids returned as strings instead. More about Twitter IDs, JSON and Snowflake.

Example Values: true

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = FS_INCOMING;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_cursor(api, &uri, cursor);
	add_stringify_ids(api, &uri, stringify_ids);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_fs_outgoing (
	char **res, //response
	cursor_t cursor, //optional. if not 0, add it to argument.
	int stringify_ids //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/friendships/outgoing.format
Parameters
cursor semi-optional

Causes the list of connections to be broken into pages of no more than 5000 IDs at a time. The number of IDs returned is not guaranteed to be 5000 as suspended users are filtered out after connections are queried. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

stringify_ids optional

Many programming environments will not consume our Tweet ids due to their size. Provide this option to have ids returned as strings instead. More about Twitter IDs, JSON and Snowflake.

Example Values: true

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = FS_OUTGOING;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_cursor(api, &uri, cursor);
	add_stringify_ids(api, &uri, stringify_ids);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int post_fs_create (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	int follow //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/friendships/create.json
Parameters

Providing either screen_name or user_id is required.
screen_name optional

The screen name of the user for whom to befriend.

Example Values: noradio

user_id optional

The ID of the user for whom to befriend.

Example Values: 12345

follow optional

Enable notifications for the target user.

Example Values: true

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!(user_id || (screen_name && screen_name[0]))) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = FS_CREATE;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_user_id(api, &uri, user_id);
	add_screen_name(api, &uri, screen_name);
	add_follow(api, &uri, follow);

	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

int post_fs_destroy (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name //optional. if not 0, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/friendships/destroy.json
Parameters

Providing either screen_name or user_id is required.

screen_name optional

The screen name of the user for whom to unfollow.

Example Values: noradio

user_id optional

The ID of the user for whom to unfollow.

Example Values: 12345
*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!(user_id || (screen_name && screen_name[0]))) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = FS_DESTROY;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_user_id(api, &uri, user_id);
	add_screen_name(api, &uri, screen_name);

	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

int post_fs_update (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	int device, //optional. if not -1, add it to argument.
	int retweets //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/friendships/update.json
Parameters

Providing either screen_name or user_id is required.

screen_name optional

The screen name of the user for whom to befriend.

Example Values: noradio

user_id optional

The ID of the user for whom to befriend.

Example Values: 12345

device optional

Enable/disable device notifications from the target user.

Example Values: true, false

retweets optional

Enable/disable retweets from the target user.

Example Values: true, false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!(user_id || (screen_name && screen_name[0]))) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = FS_UPDATE;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_user_id(api, &uri, user_id);
	add_screen_name(api, &uri, screen_name);
	add_device(api, &uri, device);
	add_retweets(api, &uri, retweets);

	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

int get_fs_show (
	char **res, //response
	tweet_id_t source_id, //optional. if not 0, add it to argument.
	char *source_screen_name, //optional. if not 0, add it to argument.
	tweet_id_t target_id, //optional. if not 0, add it to argument.
	char *target_screen_name //optional. if not 0, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/friendships/show.json
Parameters

At least one source and one target, whether specified by IDs or screen_names, should be provided to this method.

source_id optional

The user_id of the subject user.

Example Values: 3191321

source_screen_name optional

The screen_name of the subject user.

Example Values: raffi

target_id optional

The user_id of the target user.

Example Values: 20

target_screen_name optional

The screen_name of the target user.

Example Values: noradio

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!(source_id || (source_screen_name && source_screen_name[0])) && !(target_id || (target_screen_name && target_screen_name[0]))) {
		fprintf(stderr, "At least one source and one target, whether specified by IDs or screen_names");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = FS_SHOW;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_source_id(api, &uri, source_id);
	add_source_screen_name(api, &uri, source_screen_name);
	add_target_id(api, &uri, target_id);
	add_target_screen_name(api, &uri, target_screen_name);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_friends_list (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	cursor_t cursor, //optional. if not 0, add it to argument.
	int count, //optional. if not 0, add it to argument.
	int skip_status, //optional. if not -1, add it to argument.
	int include_user_entities //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/friends/list.json
Parameters

Either a screen_name or a user_id should be provided.

user_id optional

The ID of the user for whom to return results for.

Example Values: 12345

screen_name optional

The screen name of the user for whom to return results for.

Example Values: noradio

cursor semi-optional

Causes the results to be broken into pages. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

countoptional

The number of users to return per page, up to a maximum of 200. Defaults to 20.

Example Values: 42

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

Example Values: false

include_user_entities optional

The user object entities node will be disincluded when set to false.

Example Values: false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!(user_id || (screen_name && screen_name[0]))) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = FRIENDS_LIST;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_user_id(api, &uri, user_id);
	add_screen_name(api, &uri, screen_name);
	add_cursor(api, &uri, cursor);
	add_count(api, &uri, count);
	add_skip_status(api, &uri, skip_status);
	add_include_user_entities(api, &uri, include_user_entities);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_followers_list (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	cursor_t cursor, //optional. if not 0, add it to argument.
	int count, //optional. if not 0, add it to argument.
	int skip_status, //optional. if not -1, add it to argument.
	int include_user_entities //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/followers/list.json
Parameters

Either a screen_name or a user_id should be provided.

user_id optional

The ID of the user for whom to return results for.

Example Values: 12345

screen_name optional

The screen name of the user for whom to return results for.

Example Values: noradio

cursor semi-optional

Causes the results to be broken into pages. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

count optional

The number of users to return per page, up to a maximum of 200. Defaults to 20.

Example Values: 42

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

Example Values: false

include_user_entities optional

The user object entities node will be disincluded when set to false.

Example Values: false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!(user_id || (screen_name && screen_name[0]))) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = FOLLOWERS_LIST;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_user_id(api, &uri, user_id);
	add_screen_name(api, &uri, screen_name);
	add_cursor(api, &uri, cursor);
	add_count(api, &uri, count);
	add_skip_status(api, &uri, skip_status);
	add_include_user_entities(api, &uri, include_user_entities);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_account_settings (
	char **res //response
	) {
/*
Resource URL
https://api.twitter.com/1.1/account/settings.json

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = ACCOUNT_SETTINGS;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_account_verify_credentials (
	char **res, //response
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/account/verify_credentials.json
Parameters

include_entities optional

The entities node will not be included when set to false.

Example Values: false

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

Example Values: true

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = ACCOUNT_VERIFY_CREDEBTIALS;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_include_entities(api, &uri, include_entities);
	add_skip_status(api, &uri, skip_status);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int post_account_settings (
	char **res, //response
	int trend_location_woeid, //optional. if not 0, add it to argument.
	int sleep_time_enabled, //optional. if not -1, add it to argument.
	int start_sleep_time, //optional. if not -1, add it to argument.
	int end_sleep_time, //optional. if not -1, add it to argument.
	char *time_zone, //optional. if it is valid, add it to argument.
	char *lang //optional. if it is valid, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/account/settings.json
Parameters

While all parameters for this method are optional, at least one or more should be provided when executing this request.

trend_location_woeid optional

The Yahoo! Where On Earth ID to use as the user's default trend location. Global information is available by using 1 as the WOEID. The woeid must be one of the locations returned by GET trends/available.

Example Values: 1

sleep_time_enabled optional

When set to true, t or 1, will enable sleep time for the user. Sleep time is the time when push or SMS notifications should not be sent to the user.

Example Values: true

start_sleep_time optional

The hour that sleep time should begin if it is enabled. The value for this parameter should be provided in ISO8601 format (i.e. 00-23). The time is considered to be in the same timezone as the user's time_zone setting.

Example Values: 13

end_sleep_time optional

The hour that sleep time should end if it is enabled. The value for this parameter should be provided in ISO8601 format (i.e. 00-23). The time is considered to be in the same timezone as the user's time_zone setting.

Example Values: 13

time_zone optional

The timezone dates and times should be displayed in for the user. The timezone must be one of the Rails TimeZone names.

Example Values: Europe/Copenhagen, Pacific/Tongatapu

lang optional

The language which Twitter should render in for this user. The language must be specified by the appropriate two letter ISO 639-1 representation. Currently supported languages are provided by GET help/languages.

Example Values: it, en, es

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = ACCOUNT_SETTINGS;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_trend_location_woeid(api, &uri, trend_location_woeid);
	add_sleep_time_enabled(api, &uri, sleep_time_enabled);
	add_start_sleep_time(api, &uri, start_sleep_time);
	add_end_sleep_time(api, &uri, end_sleep_time);
	add_time_zone(api, &uri, time_zone);
	add_lang(api, &uri, lang);

	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

int post_account_update_delivery_device (
	char *device, //required.
	char **res, //response
	int include_entities //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/account/update_delivery_device.json
Parameters
device required

Must be one of: sms, none.

Example Values: sms

include_entities optional

When set to either true, t or 1, each tweet will include a node called "entities,". This node offers a variety of metadata about the tweet in a discreet structure, including: user_mentions, urls, and hashtags. While entities are opt-in on timelines at present, they will be made a default component of output in the future. See Tweet Entities for more detail on entities.

Example Values: true

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = ACCOUNT_UPDATE_DELIVERY_DEVICE;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_device_str(api, &uri, device);
	add_include_entities(api, &uri, include_entities);

	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

int post_account_update_profile (
	char **res, //response
	char *name, //optional. if not 0, add it to argument.
	char *url, //optional. if not 0, add it to argument.
	char *location, //optional. if not 0, add it to argument.
	char *description, //optional. if not 0, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/account/update_profile.json
Parameters

While no specific parameter is required, at least one of these parameters should be provided when executing this method.

name optional

Full name associated with the profile. Maximum of 20 characters.

Example Values: Marcel Molina

url optional

URL associated with the profile. Will be prepended with "http://" if not present. Maximum of 100 characters.

Example Values: http://project.ioni.st

location optional

The city or country describing where the user of the account is located. The contents are not normalized or geocoded in any way. Maximum of 30 characters.

Example Values: San Francisco, CA

description optional

A description of the user owning the account. Maximum of 160 characters.

Example Values: Flipped my wig at age 22 and it never grew back. Also: I work at Twitter.

include_entities optional

The entities node will not be included when set to false.

Example Values: false

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.
*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = ACCOUNT_UPDATE_PROFILE;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_name(api, &uri, name);
	add_url_upto_100(api, &uri, url);
	add_location(api, &uri, location);
	add_description(api, &uri, description);
	add_include_entities(api, &uri, include_entities);
	add_skip_status(api, &uri, skip_status);

	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

// POST account/update_profile_background_image is too difficult to implement

int post_account_update_profile_colors (
	char **res, //response
	long profile_background_color, //optional. if not -1, add it to argument.
	long profile_link_color, //optional. if not -1, add it to argument.
	long profile_sidebar_border_color, //optional. if not -1, add it to argument.
	long profile_sidebar_fill_color, //optional. if not -1, add it to argument.
	long profile_text_color, //optional. if not -1, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/account/update_profile_colors.json
Parameters
profile_background_color optional

Profile background color.

Example Values: 3D3D3D

profile_link_color optional

Profile link color.

Example Values: 0000FF

profile_sidebar_border_color optional

Profile sidebar's border color.

Example Values: 0F0F0F

profile_sidebar_fill_color optional

Profile sidebar's background color.

Example Values: 00FF00

profile_text_color optional

Profile text color.

Example Values: 000000

include_entities optional

The entities node will not be included when set to false.

Example Values: false

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = ACCOUNT_UPDATE_PROFILE_COLORS;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_profile_background_color(api, &uri, profile_background_color);
	add_profile_link_color(api, &uri, profile_link_color);
	add_profile_sidebar_border_color(api, &uri, profile_sidebar_border_color);
	add_profile_sidebar_fill_color(api, &uri, profile_sidebar_fill_color);
	add_profile_text_color(api, &uri, profile_text_color);
	add_include_entities(api, &uri, include_entities);
	add_skip_status(api, &uri, skip_status);

	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

// POST account/update_profile_image is too difficult to implement

int get_blocks_list (
	char **res, //response
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status, //optional. if not -1, add it to argument.
	cursor_t cursor //optional. if not 0, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/blocks/list.json
Parameters
include_entities optional

The entities node will not be included when set to false.

Example Values: false

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

cursor semi-optional

Causes the list of blocked users to be broken into pages of no more than 5000 IDs at a time. The number of IDs returned is not guaranteed to be 5000 as suspended users are filtered out after connections are queried. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = BLOCKS_LIST;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_include_entities(api, &uri, include_entities);
	add_skip_status(api, &uri, skip_status);
	add_cursor(api, &uri, cursor);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_blocks_ids (
	char **res, //response
	int stringify_ids, //optional. if not -1, add it to argument.
	cursor_t cursor //optional. if not 0, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/blocks/ids.json
Parameters
stringify_ids optional

Many programming environments will not consume our ids due to their size. Provide this option to have ids returned as strings instead. Read more about Twitter IDs, JSON and Snowflake.

Example Values: true

cursor semi-optional

Causes the list of IDs to be broken into pages of no more than 5000 IDs at a time. The number of IDs returned is not guaranteed to be 5000 as suspended users are filtered out after connections are queried. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = BLOCKS_IDS;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_stringify_ids(api, &uri, stringify_ids);
	add_cursor(api, &uri, cursor);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int post_blocks_create (
	char **res, //response
	char *screen_name, //optional. if not 0, add it to argument.
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/blocks/create.json
Parameters

Either screen_name or user_id must be provided.
screen_name optional

The screen name of the potentially blocked user. Helpful for disambiguating when a valid screen name is also a user ID.

Example Values: noradio
user_id optional

The ID of the potentially blocked user. Helpful for disambiguating when a valid user ID is also a valid screen name.

Example Values: 12345
include_entities optional

The entities node will not be included when set to false.

Example Values: false
skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = BLOCKS_CREATE;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_screen_name(api, &uri, screen_name);
	add_user_id(api, &uri, user_id);
	add_include_entities(api, &uri, include_entities);
	add_skip_status(api, &uri, skip_status);

	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

int post_blocks_destroy (
	char **res, //response
	char *screen_name, //optional. if not 0, add it to argument.
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/blocks/destroy.json
Parameters

One of screen_name or id must be provided.

screen_name optional

The screen name of the potentially blocked user. Helpful for disambiguating when a valid screen name is also a user ID.

Example Values: noradio

user_id optional

The ID of the potentially blocked user. Helpful for disambiguating when a valid user ID is also a valid screen name.

Example Values: 12345

include_entities optional

The entities node will not be included when set to false.

Example Values: false

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = BLOCKS_DESTROY;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_screen_name(api, &uri, screen_name);
	add_user_id(api, &uri, user_id);
	add_include_entities(api, &uri, include_entities);
	add_skip_status(api, &uri, skip_status);

	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

int get_users_lookup (
	char **res, //response
	char *screen_name, //optional. if not 0, add it to argument.
	char *user_id, //optional. if not 0, add it to argument.
	int include_entities //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/users/lookup.json
Parameters
screen_name optional

A comma separated list of screen names, up to 100 are allowed in a single request. You are strongly encouraged to use a POST for larger (up to 100 screen names) requests.

Example Values: twitterapi,twitter

user_id optional

A comma separated list of user IDs, up to 100 are allowed in a single request. You are strongly encouraged to use a POST for larger requests.

Example Values: 783214,6253282

include_entities optional

The entities node that may appear within embedded statuses will be disincluded when set to false.

Example Values: false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = USERS_LOOKUP;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_screen_name(api, &uri, screen_name);
	add_user_id_str(api, &uri, user_id);
	add_include_entities(api, &uri, include_entities);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_users_show (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	int include_entities //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/users/show.json
Parameters

user_id required

The ID of the user for whom to return results for. Either an id or screen_name is required for this method.

Example Values: 12345

screen_name required

The screen name of the user for whom to return results for. Either a id or screen_name is required for this method.

Example Values: noradio

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = USERS_SHOW;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_user_id(api, &uri, user_id);
	add_screen_name(api, &uri, screen_name);
	add_include_entities(api, &uri, include_entities);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_users_search (
		char *q, //required.
	char **res, //response
	int page, //optional. if not 0, add it to argument.
	int count, //optional. if not 0, add it to argument.
	int include_entities //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/users/search.json
Parameters
q required

The search query to run against people search.

Example Values: Twitter%20API

page optional

Specifies the page of results to retrieve.

Example Values: 3

count optional

The number of potential user results to retrieve per page. This value has a maximum of 20.

Example Values: 5

include_entities optional

The entities node will be disincluded from embedded tweet objects when set to false.

Example Values: false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = USERS_SEARCH;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_q(api, &uri, q);
	add_page(api, &uri, page);
	add_count_upto_20(api, &uri, count);
	add_include_entities(api, &uri, include_entities);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_users_contributees (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/users/contributees.json
Parameters

A user_id or screen_name is required.
user_id optional

The ID of the user for whom to return results for. Helpful for disambiguating when a valid user ID is also a valid screen name.
screen_name optional

The screen name of the user for whom to return results for.
include_entities optional

The entities node will be disincluded when set to false.

Example Values: false
skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = USERS_CONTRIBUTEES;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_user_id(api, &uri, user_id);
	add_screen_name(api, &uri, screen_name);
	add_include_entities(api, &uri, include_entities);
	add_skip_status(api, &uri, skip_status);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int get_users_contributors (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/users/contributors.json
Parameters

A user_id or screen_name is required.

user_id optional

The ID of the user for whom to return results for.

screen_name optional

The screen name of the user for whom to return results for.

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = USERS_CONTRIBUTORS;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_user_id(api, &uri, user_id);
	add_screen_name(api, &uri, screen_name);
	add_include_entities(api, &uri, include_entities);
	add_skip_status(api, &uri, skip_status);

	int ret = http_request(uri, GET, res);

	free(uri);uri = NULL;

	return ret;
}

int post_account_remove_profile_banner (
	char **res //response
	) {
/*
Resource URL
https://api.twitter.com/1.1/account/remove_profile_banner.json
*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = ACCOUNT_REMOVE_PROFILE_BANNER;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

// POST account/update_profile_banner is too difficult to implement

int get_users_profile_banner (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name //optional. if not 0, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/users/profile_banner.json
Parameters

Always specify either an user_id or screen_name when requesting this method.

user_id optional

The ID of the user for whom to return results for. Helpful for disambiguating when a valid user ID is also a valid screen name.

Example Values: 12345

Note:: Specifies the ID of the user to befriend. Helpful for disambiguating when a valid user ID is also a valid screen name.

screen_name optional

The screen name of the user for whom to return results for. Helpful for disambiguating when a valid screen name is also a user ID.

Example Values: noradio

*/
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	enum APIS api = USERS_PROFILE_BANNER;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_user_id(api, &uri, user_id);
	add_screen_name(api, &uri, screen_name);

	int ret = http_request(uri, POST, res);

	free(uri);uri = NULL;

	return ret;
}

