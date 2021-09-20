/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.onboarding;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Build;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.TextPaint;
import android.text.method.LinkMovementMethod;
import android.text.style.ClickableSpan;
import android.text.style.ForegroundColorSpan;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;

import androidx.appcompat.widget.AppCompatImageView;

import org.chromium.base.SysUtils;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.BraveRewardsHelper;
import org.chromium.chrome.browser.ChromeTabbedActivity;
import org.chromium.chrome.browser.app.BraveActivity;
import org.chromium.chrome.browser.customtabs.CustomTabActivity;

public class BraveTalkOptInPopup {
    private final View mAnchor;
    private final BraveTalkOptInPopupListener mListener;
    private final LayoutInflater mInflater;
    private final ChromeTabbedActivity mActivity;
    private final PopupWindow mWindow;
    private final View mView;
    private final AppCompatImageView mOptInImage;
    private final Button mOptInButton;
    private final TextView mOptInPopupTitle;
    private final TextView mOptInPopupDescription;
    private final TextView mOptInPopupTos;

    public BraveTalkOptInPopup(View anchor, BraveTalkOptInPopupListener listener) {
        mAnchor = anchor;
        mListener = listener;
        mInflater = (LayoutInflater) mAnchor.getContext().getSystemService(
                Context.LAYOUT_INFLATER_SERVICE);
        mActivity = BraveRewardsHelper.getChromeTabbedActivity();
        mWindow = createPopupWindow();

        mView = mInflater.inflate(R.layout.brave_talk_opt_in_layout, null);
        mWindow.setContentView(mView);

        mOptInImage = mView.findViewById(R.id.brave_talk_opt_in_image);
        mOptInButton = mView.findViewById(R.id.brave_talk_opt_in_button);
        mOptInPopupTitle = mView.findViewById(R.id.brave_talk_opt_in_title);
        mOptInPopupDescription = mView.findViewById(R.id.brave_talk_opt_in_description);
        mOptInPopupTos = mView.findViewById(R.id.brave_talk_opt_in_tos);

        initPopupContent();
    }

    public void showLikePopDownMenu() {
        mWindow.setAnimationStyle(R.style.OverflowMenuAnim);

        if (SysUtils.isLowEndDevice()) {
            mWindow.setAnimationStyle(0);
        }

        mWindow.showAsDropDown(mAnchor, /* xOffset */ 0, /* yOffset */ 0);
    }

    public void dismissPopup() {
        mWindow.dismiss();
    }

    private PopupWindow createPopupWindow() {
        PopupWindow window = new PopupWindow(mAnchor.getContext());
        window.setWidth(ViewGroup.LayoutParams.WRAP_CONTENT);
        window.setHeight(ViewGroup.LayoutParams.WRAP_CONTENT);
        window.setBackgroundDrawable(new ColorDrawable(Color.WHITE));
        window.setTouchable(true);
        window.setFocusable(true);
        window.setOutsideTouchable(true);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            window.setElevation(20);
        }

        window.setTouchInterceptor(new View.OnTouchListener() {
            @SuppressLint("ClickableViewAccessibility")
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_OUTSIDE) {
                    dismissPopup();
                    return true;
                }
                return false;
            }
        });
        window.setOnDismissListener(new PopupWindow.OnDismissListener() {
            @Override
            public void onDismiss() {
                BraveActivity braveActivity = BraveRewardsHelper.getBraveActivity();
                if (braveActivity != null) {
                    braveActivity.onBraveTalkOptInPopupDismiss();
                }

                mListener.notifyTalkOptInPopupClosed();
            }
        });

        return window;
    }

    private void initPopupContent() {
        mOptInButton.setOnClickListener((new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onBraveTalkOptInButtonClicked();
            }
        }));

        if (BraveRewardsHelper.shouldShowBraveRewardsOnboardingModal()) {
            initTosTextView();
        } else {
            mOptInButton.setText(mView.getResources().getString(
                    R.string.brave_talk_turn_on_private_ads_button_text));
            mOptInPopupTitle.setText(
                    mView.getResources().getString(R.string.brave_talk_turn_on_private_ads_text));
            mOptInPopupDescription.setText(mView.getResources().getString(
                    R.string.brave_talk_private_ads_description_text));
            mOptInPopupTos.setVisibility(View.GONE);
        }
    }

    private void initTosTextView() {
        String tosText = String.format(
                mView.getResources().getString(R.string.brave_talk_rewards_tos_pp_text),
                mView.getResources().getString(R.string.terms_of_service),
                mView.getResources().getString(R.string.privacy_policy));
        int termsOfServiceIndex =
                tosText.indexOf(mView.getResources().getString(R.string.terms_of_service));
        Spanned tosTextSpanned = BraveRewardsHelper.spannedFromHtmlString(tosText);
        SpannableString tosTextSS = new SpannableString(tosTextSpanned.toString());

        ClickableSpan tosClickableSpan = new ClickableSpan() {
            @Override
            public void onClick(View textView) {
                CustomTabActivity.showInfoPage(mActivity, BraveActivity.BRAVE_TERMS_PAGE);
            }
            @Override
            public void updateDrawState(TextPaint ds) {
                super.updateDrawState(ds);
                ds.setUnderlineText(true);
            }
        };

        tosTextSS.setSpan(tosClickableSpan, termsOfServiceIndex,
                termsOfServiceIndex
                        + mActivity.getResources().getString(R.string.terms_of_service).length(),
                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        tosTextSS.setSpan(new ForegroundColorSpan(mView.getResources().getColor(
                                  R.color.brave_rewards_modal_theme_color)),
                termsOfServiceIndex,
                termsOfServiceIndex
                        + mActivity.getResources().getString(R.string.terms_of_service).length(),
                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);

        ClickableSpan privacyProtectionClickableSpan = new ClickableSpan() {
            @Override
            public void onClick(View textView) {
                CustomTabActivity.showInfoPage(mActivity, BraveActivity.BRAVE_PRIVACY_POLICY);
            }
            @Override
            public void updateDrawState(TextPaint ds) {
                super.updateDrawState(ds);
                ds.setUnderlineText(true);
            }
        };

        int privacyPolicyIndex =
                tosText.indexOf(mView.getResources().getString(R.string.privacy_policy));
        tosTextSS.setSpan(privacyProtectionClickableSpan, privacyPolicyIndex,
                privacyPolicyIndex
                        + mView.getResources().getString(R.string.privacy_policy).length(),
                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        tosTextSS.setSpan(new ForegroundColorSpan(mView.getResources().getColor(
                                  R.color.brave_rewards_modal_theme_color)),
                privacyPolicyIndex,
                privacyPolicyIndex
                        + mView.getResources().getString(R.string.privacy_policy).length(),
                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);

        mOptInPopupTos.setMovementMethod(LinkMovementMethod.getInstance());
        mOptInPopupTos.setText(tosTextSS);
    }

    private void onBraveTalkOptInButtonClicked() {
        mListener.notifyAdsEnableButtonPressed();
        BraveRewardsHelper.setShowBraveRewardsOnboardingModal(false);
        BraveRewardsHelper.setShowBraveRewardsOnboardingOnce(false);
        showStartFreeCallLayout();
    }

    private void showStartFreeCallLayout() {
        mOptInImage.setImageResource(R.drawable.ic_check_circle);
        LinearLayout.LayoutParams params =
                (LinearLayout.LayoutParams) mOptInImage.getLayoutParams();
        params.topMargin = 40;
        params.bottomMargin = 40;
        mOptInImage.setLayoutParams(params);
        mOptInButton.setVisibility(View.GONE);
        mOptInPopupTitle.setText(
                mView.getResources().getString(R.string.brave_talk_start_free_call_text));
        mOptInPopupDescription.setText(
                mView.getResources().getString(R.string.brave_talk_click_anywhere_text));
        mOptInPopupTos.setVisibility(View.VISIBLE);
        mOptInPopupTos.setTextSize(14);
        showQuickTourTextLink();
    }

    private void showQuickTourTextLink() {
        String optInString = String.format(
                mActivity.getResources().getString(R.string.brave_talk_want_learn_more_text,
                        mActivity.getResources().getString(R.string.quick_tour)));
        int quickTourIndex =
                optInString.indexOf(mActivity.getResources().getString(R.string.quick_tour));
        Spanned optInTextSpanned = BraveRewardsHelper.spannedFromHtmlString(optInString);
        SpannableString optInTextSS = new SpannableString(optInTextSpanned.toString());

        ClickableSpan optInClickableSpan = new ClickableSpan() {
            @Override
            public void onClick(View textView) {
                onQuickTourClicked();
            }
            @Override
            public void updateDrawState(TextPaint ds) {
                super.updateDrawState(ds);
                ds.setUnderlineText(true);
            }
        };

        optInTextSS.setSpan(optInClickableSpan, quickTourIndex,
                quickTourIndex + mActivity.getResources().getString(R.string.quick_tour).length(),
                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        optInTextSS.setSpan(new ForegroundColorSpan(mActivity.getResources().getColor(
                                    R.color.brave_rewards_modal_theme_color)),
                quickTourIndex,
                quickTourIndex + mActivity.getResources().getString(R.string.quick_tour).length(),
                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        mOptInPopupTos.setMovementMethod(LinkMovementMethod.getInstance());
        mOptInPopupTos.setText(optInTextSS);
    }

    private void onQuickTourClicked() {
        BraveActivity braveActivity = BraveActivity.getBraveActivity();
        if (braveActivity != null) {
            braveActivity.openRewardsPanelWithQuickTour();
        }
    }
}
