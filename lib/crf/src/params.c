#include <os.h>

#include <stdlib.h>
#include <string.h>

#include <crf.h>
#include "quark.h"

enum {
	PT_NONE = 0,
	PT_INT,
	PT_FLOAT,
	PT_STRING,
};

typedef struct {
	char*	name;
	int		type;
	int		val_i;
	float_t val_f;
	char*	val_s;
} param_t;

typedef struct {
	int num_params;
	param_t* params;
} params_t;


static int params_addref(crf_params_t* params)
{
	return crf_interlocked_increment(&params->nref);
}

static int params_release(crf_params_t* params)
{
	int count = crf_interlocked_decrement(&params->nref);
	if (count == 0) {
		int i;
		params_t* pars = (params_t*)params->internal;
		for (i = 0;i < pars->num_params;++i) {
			free(pars->params[i].name);
			free(pars->params[i].val_s);
		}
		free(pars);
	}
	return count;
}

static param_t* find_param(params_t* pars, const char *name)
{
	int i;

	for (i = 0;i < pars->num_params;++i) {
		if (strcmp(pars->params[i].name, name) == 0) {
			return &pars->params[i];
		}
	}

	return NULL;
}

static int params_set(crf_params_t* params, const char *name, const char *value)
{
	params_t* pars = (params_t*)params->internal;
	param_t* par = find_param(pars, name);
	if (par == NULL) return -1;
	switch (par->type) {
	case PT_INT:
		par->val_i = (value != NULL) ? atoi(value) : 0;
		break;
	case PT_FLOAT:
		par->val_f = (value != NULL) ? (float_t)atof(value) : 0;
		break;
	case PT_STRING:
		free(par->val_s);
		par->val_s = (value != NULL) ? strdup(value) : strdup("");
	}
	return 0;
}

static int params_set_int(crf_params_t* params, const char *name, int value)
{
	params_t* pars = (params_t*)params->internal;
	param_t* par = find_param(pars, name);
	if (par == NULL) return -1;
	if (par->type != PT_INT) return -1;
	par->val_i = value;
	return 0;
}

static int params_set_float(crf_params_t* params, const char *name, float_t value)
{
	params_t* pars = (params_t*)params->internal;
	param_t* par = find_param(pars, name);
	if (par == NULL) return -1;
	if (par->type != PT_FLOAT) return -1;
	par->val_f = value;
	return 0;
}

static int params_set_string(crf_params_t* params, const char *name, const char *value)
{
	params_t* pars = (params_t*)params->internal;
	param_t* par = find_param(pars, name);
	if (par == NULL) return -1;
	if (par->type != PT_STRING) return -1;
	free(par->val_s);
	par->val_s = strdup(value);
	return 0;
}

static int params_get_int(crf_params_t* params, const char *name, int *value)
{
	params_t* pars = (params_t*)params->internal;
	param_t* par = find_param(pars, name);
	if (par == NULL) return -1;
	if (par->type != PT_INT) return -1;
	*value = par->val_i;
	return 0;
}

static int params_get_float(crf_params_t* params, const char *name, float_t *value)
{
	params_t* pars = (params_t*)params->internal;
	param_t* par = find_param(pars, name);
	if (par == NULL) return -1;
	if (par->type != PT_FLOAT) return -1;
	*value = par->val_f;
	return 0;
}

static int params_get_string(crf_params_t* params, const char *name, char **value)
{
	params_t* pars = (params_t*)params->internal;
	param_t* par = find_param(pars, name);
	if (par == NULL) return -1;
	if (par->type != PT_STRING) return -1;
	*value = par->val_s;
	return 0;
}

crf_params_t* params_create_instance()
{
	crf_params_t* params = (crf_params_t*)calloc(1, sizeof(crf_params_t));

	if (params != NULL) {
		/* Construct the internal data. */
		params->internal = (params_t*)calloc(1, sizeof(params_t));
		if (params->internal == NULL) {
			free(params);
			return NULL;
		}

		/* Set member functions. */
		params->nref = 1;
		params->addref = params_addref;
		params->release = params_release;
		params->set = params_set;
		params->set_int = params_set_int;
		params->set_float = params_set_float;
		params->set_string = params_set_string;
		params->get_int = params_get_int;
		params->get_float = params_get_float;
		params->get_string = params_get_string;
	}

	return params;
}

int params_add_int(crf_params_t* params, const char *name, int value)
{
	param_t* par = NULL;
	params_t* pars = (params_t*)params->internal;
	pars->params = (param_t*)realloc(pars->params, (pars->num_params+1) * sizeof(param_t));
	if (pars->params == NULL) {
		return -1;
	}

	par = &pars->params[pars->num_params++];
	memset(par, 0, sizeof(*par));
	par->name = strdup(name);
	par->type = PT_INT;
	par->val_i = value;
	return 0;
}

int params_add_float(crf_params_t* params, const char *name, float_t value)
{
	param_t* par = NULL;
	params_t* pars = (params_t*)params->internal;
	pars->params = (param_t*)realloc(pars->params, (pars->num_params+1) * sizeof(param_t));
	if (pars->params == NULL) {
		return -1;
	}

	par = &pars->params[pars->num_params++];
	memset(par, 0, sizeof(*par));
	par->name = strdup(name);
	par->type = PT_FLOAT;
	par->val_f = value;
	return 0;
}

int params_add_string(crf_params_t* params, const char *name, const char *value)
{
	param_t* par = NULL;
	params_t* pars = (params_t*)params->internal;
	pars->params = (param_t*)realloc(pars->params, (pars->num_params+1) * sizeof(param_t));
	if (pars->params == NULL) {
		return -1;
	}

	par = &pars->params[pars->num_params++];
	memset(par, 0, sizeof(*par));
	par->name = strdup(name);
	par->type = PT_STRING;
	par->val_s = strdup(value);
	return 0;
}
